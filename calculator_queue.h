#ifndef CALCULATOR_QUEUE_H
#define CALCULATOR_QUEUE_H

#include <wrapper/Aeron.h>
#include <wrapper/Publication.h>
#include <wrapper/Subscription.h>
#include <wrapper/concurrent/AtomicBuffer.h>
#include <concurrent/ringbuffer/ManyToOneRingBuffer.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <queue>
#include <atomic>
#include <limits>
#include <algorithm>
#include "calculator_functions.h"

using namespace aeron;
using namespace aeron::concurrent;
using namespace aeron::concurrent::ringbuffer;

struct CalculatorCommand {
    double num1;
    double num2;
    char op;
    uint64_t timestamp;
};

class CalculatorQueue {
private:
    static constexpr int COMMAND_BUFFER_SIZE = 1024;
    static constexpr int COMMAND_HEADER_LENGTH = sizeof(CalculatorCommand);
    static constexpr size_t MAX_RESULTS = 1000000;
    static constexpr size_t MAX_LATENCIES = 1000000;
    
    std::shared_ptr<Aeron> aeron;
    std::shared_ptr<Publication> publication;
    std::shared_ptr<Subscription> subscription;
    std::vector<std::uint8_t> buffer;
    AtomicBuffer atomicBuffer;
    ManyToOneRingBuffer commandBuffer;
    std::thread processingThread;
    std::atomic<bool> running{true};
    std::mutex mutex;
    std::condition_variable cv;
    std::vector<double> results;
    std::vector<uint64_t> latencies;
    std::atomic<size_t> totalCommandsProcessed{0};
    std::atomic<size_t> totalLatency{0};
    std::atomic<uint64_t> minLatency{std::numeric_limits<uint64_t>::max()};
    std::atomic<uint64_t> maxLatency{0};
    
    void processCommands() {
        while (running) {
            int messagesRead = 0;
            commandBuffer.read([&](int32_t msgTypeId, concurrent::AtomicBuffer& buffer, int index, int length) {
                if (msgTypeId == 1) {
                    CalculatorCommand* cmd = reinterpret_cast<CalculatorCommand*>(buffer.buffer() + index);
                    auto start = std::chrono::high_resolution_clock::now();
                    
                    try {
                        double result = calculate(cmd->num1, cmd->num2, cmd->op);
                        
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            if (results.size() < MAX_RESULTS) {
                                results.push_back(result);
                            }
                        }
                        
                        auto end = std::chrono::high_resolution_clock::now();
                        uint64_t latency = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
                        
                        totalCommandsProcessed++;
                        totalLatency += latency;
                        
                        {
                            std::lock_guard<std::mutex> lock(mutex);
                            if (latencies.size() < MAX_LATENCIES) {
                                latencies.push_back(latency);
                            }
                        }
                        
                        uint64_t currentMin = minLatency.load();
                        while (latency < currentMin && !minLatency.compare_exchange_weak(currentMin, latency)) {
                            // Retry if another thread updated minLatency
                        }
                        
                        uint64_t currentMax = maxLatency.load();
                        while (latency > currentMax && !maxLatency.compare_exchange_weak(currentMax, latency)) {
                            // Retry if another thread updated maxLatency
                        }
                    } catch (const std::runtime_error& e) {
                        std::cerr << "Error processing command: " << e.what() << std::endl;
                    }
                    
                    messagesRead++;
                }
                return true;
            });
            
            if (messagesRead == 0) {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait_for(lock, std::chrono::milliseconds(100));
            }
        }
    }

    void cleanup() {
        std::lock_guard<std::mutex> lock(mutex);
        results.clear();
        results.shrink_to_fit();
        latencies.clear();
        latencies.shrink_to_fit();
        
        totalCommandsProcessed = 0;
        totalLatency = 0;
        minLatency = std::numeric_limits<uint64_t>::max();
        maxLatency = 0;
    }

public:
    CalculatorQueue() 
        : buffer(COMMAND_BUFFER_SIZE)
        , atomicBuffer(buffer.data(), buffer.size())
        , commandBuffer(atomicBuffer) {
        // Set up Aeron context
        Context ctx;
        ctx.aeronDir("/tmp/aeron-benjamin.hanson");
        
        aeron = Aeron::connect(ctx);
        
        // Setup publication and subscription
        const std::string channel = "aeron:ipc";
        const int streamId = 10;
        
        std::int64_t pubRegId = aeron->addPublication(channel, streamId);
        std::int64_t subRegId = aeron->addSubscription(channel, streamId);
        
        // Wait for publication and subscription to be established
        while (!publication) {
            publication = aeron->findPublication(pubRegId);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        while (!subscription) {
            subscription = aeron->findSubscription(subRegId);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        processingThread = std::thread(&CalculatorQueue::processCommands, this);
    }
    
    ~CalculatorQueue() {
        running = false;
        cv.notify_all();
        if (processingThread.joinable()) {
            processingThread.join();
        }
        cleanup();
    }
    
    void submitCommand(double num1, double num2, char op) {
        CalculatorCommand cmd{num1, num2, op, 
            static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count())};
        
        AtomicBuffer buffer(reinterpret_cast<uint8_t*>(&cmd), sizeof(cmd));
        while (!commandBuffer.write(1, buffer, 0, sizeof(cmd))) {
            std::this_thread::yield();
        }
        cv.notify_one();
    }
    
    void printMetrics() {
        if (totalCommandsProcessed == 0) {
            std::cout << "No commands processed yet." << std::endl;
            return;
        }
        
        double avgLatency = static_cast<double>(totalLatency) / totalCommandsProcessed;
        
        std::cout << "\nPerformance Metrics:" << std::endl;
        std::cout << "Total commands processed: " << totalCommandsProcessed << std::endl;
        std::cout << "Average latency: " << avgLatency << " ns" << std::endl;
        std::cout << "Min latency: " << minLatency << " ns" << std::endl;
        std::cout << "Max latency: " << maxLatency << " ns" << std::endl;
        
        cleanup();
    }
    
    const std::vector<double>& getResults() const {
        return results;
    }
};

#endif // CALCULATOR_QUEUE_H 