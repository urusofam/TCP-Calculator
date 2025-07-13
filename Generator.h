//
// Created by urus on 13.07.25.
//

#ifndef TCP_CALCULATOR_GENERATOR_H
#define TCP_CALCULATOR_GENERATOR_H

#include <random>

class Generator {
    std::mt19937 gen;
    std::uniform_int_distribution<> numDistrib;
    std::uniform_int_distribution<> operatorsDistrib;

public:
    Generator();
    std::pair<std::string, double> generate(int);
};


#endif //TCP_CALCULATOR_GENERATOR_H
