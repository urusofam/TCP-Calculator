//
// Created by urus on 13.07.25.
//

#include "Generator.h"
#include "Calculator.h"
#include <chrono>

Generator::Generator() : gen(std::chrono::steady_clock::now().time_since_epoch().count()), numDistrib(1, 100), operatorsDistrib(0, 3) {}

std::pair<std::string, double> Generator::generate(int countOfNumbers) {
    std::string expression;
    std::vector<char> operators = {'+', '-', '*', '/'};

    expression += std::to_string(numDistrib(gen));
    for (int i = 0; i < countOfNumbers - 1; i++) {
        expression += operators[operatorsDistrib(gen)];
        expression += std::to_string(numDistrib(gen));
    }

    Calculator calculator;
    double result = calculator.calculate(expression);
    expression += ' ';

    return {expression, result};
}