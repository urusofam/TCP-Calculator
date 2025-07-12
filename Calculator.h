//
// Created by urus on 12.07.25.
//

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <stack>
#include <string>

class Calculator {
    int getPriority(char);
    bool isOperation(char);
	double applyOperation(std::stack<double>&, std::stack<char>&);
public:
	double calculate(const std::string&);
};



#endif //CALCULATOR_H
