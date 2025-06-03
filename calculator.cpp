#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include "calculator_queue.h"

using namespace std;
using namespace std::chrono;

void runPerformanceTest(CalculatorQueue& queue, int numOperations) {
    cout << "\nRunning performance test with " << numOperations << " operations..." << endl;
    
    auto start = high_resolution_clock::now();
    
    // Submit a mix of different operations
    for (int i = 0; i < numOperations; ++i) {
        double num1 = i + 1;
        double num2 = (i % 10) + 1;
        char op;
        
        switch (i % 4) {
            case 0: op = '+'; break;
            case 1: op = '-'; break;
            case 2: op = '*'; break;
            case 3: op = '/'; break;
        }
        
        queue.submitCommand(num1, num2, op);
    }
    
    // Wait for all operations to complete
    this_thread::sleep_for(seconds(1));
    
    auto end = high_resolution_clock::now();
    auto totalTime = duration_cast<nanoseconds>(end - start).count();
    
    cout << "\nTest completed in " << totalTime << " ns" << endl;
    cout << "Average time per operation: " << static_cast<double>(totalTime) / numOperations << " ns" << endl;
    
    queue.printMetrics();
}

int main() {
    CalculatorQueue queue;
    
    // Interactive mode
    cout << "Calculator Queue System" << endl;
    cout << "Enter 'q' to quit, 'p' for performance test, or enter calculations" << endl;
    cout << "For performance test, enter 'p' followed by number of operations (e.g., 'p 1000000')" << endl;
    
    while (true) {
        string input;
        cout << "\nEnter command (q/p/calculation): ";
        getline(cin, input);
        
        if (input.empty()) {
            continue;
        }
        
        if (input == "q") {
            break;
        } else if (input[0] == 'p') {
            int numOperations = 1000000; // Default value
            istringstream iss(input);
            string cmd;
            iss >> cmd;
            
            if (iss >> numOperations) {
                if (numOperations <= 0) {
                    cerr << "Number of operations must be positive" << endl;
                    continue;
                }
            }
            
            runPerformanceTest(queue, numOperations);
            continue;
        }
        
        try {
            double num1, num2;
            char op;
            
            istringstream iss(input);
            iss >> num1 >> op;
            
            if (op != '!') {
                iss >> num2;
            } else {
                num2 = 0;
            }
            
            queue.submitCommand(num1, num2, op);
            cout << "Command submitted to queue" << endl;
            
        } catch (const exception& e) {
            cerr << "Invalid input: " << e.what() << endl;
        }
    }
    
    return 0;
} 