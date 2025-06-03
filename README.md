# Calculator Queue System

A high-performance calculator system that uses Aeron IPC for inter-process communication.

## Prerequisites

- CMake (version 3.10 or higher)
- C++17 compatible compiler
- Aeron C++ client library installed
- Aeron media driver installed

## Building

1. Create a build directory:
```bash
mkdir build
cd build
```

2. Configure and build:
```bash
cmake ..
make
```

## Running

1. Start the Aeron media driver in a separate terminal:
```bash
aeronmd -Daeron.dir=/tmp/aeron-benjamin.hanson
```

2. Run the calculator in another terminal:
```bash
./build/calculator
```

## Usage

The calculator supports the following operations:
- Addition: `5 + 3`
- Subtraction: `10 - 4`
- Multiplication: `6 * 7`
- Division: `15 / 3`
- Factorial: `5 !` (only uses the first number)
- Exponentiation: `2 ^ 3`

Additional commands:
- Performance test: `p 1000000` (runs 1 million random operations)
- Quit: `q`

## Performance Metrics

The calculator tracks and reports:
- Total commands processed
- Average latency
- Minimum latency
- Maximum latency

## Architecture

The system consists of:
- A command queue using Aeron IPC
- A processing thread for calculations
- Performance monitoring
- Error handling for invalid operations

## Error Handling

The calculator handles:
- Division by zero
- Factorial of negative numbers
- Factorial of numbers too large (max input: 20)
- Invalid operators 