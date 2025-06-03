#ifndef CALCULATOR_FUNCTIONS_H
#define CALCULATOR_FUNCTIONS_H

#include <stdexcept>

// Function to calculate factorial
unsigned long long factorial(int n) {
    if (n < 0) {
        throw std::runtime_error("Error: Factorial is not defined for negative numbers");
    }
    if (n > 20) {  // 21! overflows unsigned long long
        throw std::runtime_error("Error: Factorial result too large (max input: 20)");
    }
    unsigned long long result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// Function to perform calculations
double calculate(double num1, double num2, char op) {
    switch(op) {
        case '+':
            return num1 + num2;
        case '-':
            return num1 - num2;
        case '*':
            return num1 * num2;
        case '/':
            if (num2 == 0) {
                throw std::runtime_error("Error: Division by zero");
            }
            return num1 / num2;
        case '!':
            return factorial(static_cast<int>(num1));
        default:
            throw std::runtime_error("Error: Invalid operator");
    }
}

#endif // CALCULATOR_FUNCTIONS_H 