//
// Created by urus on 12.07.25.
//

#include "Calculator.h"
#include <stdexcept>

int Calculator::getPriority(char operation) {
	if (operation == '+' || operation == '-')  return 1;
    else if (operation == '*' || operation == '/') return 2;
    else if (operation == '(') return 0;
    else throw std::invalid_argument("Unknown operation");
}

double Calculator::applyOperation(std::stack<double>& values, std::stack<char>& operations) {
    double b = values.top(); values.pop();
    double a = values.top(); values.pop();
    char operation = operations.top(); operations.pop();

    switch (operation) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': if (b == 0) throw std::invalid_argument("Division by zero");
                  else return a / b;
    }
    throw std::invalid_argument("Unknown operation");
}

bool Calculator::isOperation(char operation) {
    return operation == '+' || operation == '-' || operation == '*' || operation == '/';
}

double Calculator::calculate(const std::string& expression) {
    std::stack<double> values;
    std::stack<char> operations;

    for (size_t i = 0; i < expression.length(); i++) {
        if (std::isspace(expression[i])) continue;

        else if (std::isdigit(expression[i]) || (expression[i] == '-' && (i == 0 || isOperation(expression[i - 1]) || expression[i - 1] == '('))) {
            std::string number;
            if (expression[i] == '-') {
                number += '-';
                i++;
            }

            while (i < expression.length() && (std::isdigit(expression[i]) || expression[i] == '.')) {
                number += expression[i++];
            }
            i--;

            values.push(std::stod(number));
        }

        else if (isOperation(expression[i])) {
            while (!operations.empty() && getPriority(operations.top()) >= getPriority(expression[i])) {
                if (values.size() < 2) break;
                values.push(applyOperation(values, operations));
            }
            operations.push(expression[i]);
        }

        else if (expression[i] == '(') operations.push(expression[i]);
        else if (expression[i] == ')') {
            while (!operations.empty() && operations.top() != '(') {
                if (values.size() < 2) throw std::invalid_argument("Invalid expression");
                values.push(applyOperation(values, operations));
            }
            if (!operations.empty()) operations.pop();
            else throw std::invalid_argument("Invalid expression");
        }
        else throw std::invalid_argument("Invalid expression");
    }

    while (!operations.empty()) {
        if (values.size() < 2) throw std::invalid_argument("Invalid expression");
        values.push(applyOperation(values, operations));
    }

    return values.top();
}