//
// Created by urus on 13.07.25.
//

#ifndef TCP_CALCULATOR_TCPCLIENT_H
#define TCP_CALCULATOR_TCPCLIENT_H

#include "Generator.h"
#include <queue>
#include <unordered_map>

struct ConnectionState {
    int fd;
    std::string expression;
    double expected;
    std::string buffer;
    std::queue<std::string> fragments;
    bool isSendingComplete;
};

class TCPClient {
    int epollFD;
    std::unordered_map<int, ConnectionState> connections;
    Generator generator;
    std::mt19937 gen;
    static const int BUFFER_SIZE = 1024;

    void setNonBlocking(int);
    std::vector<std::string> splitExpression(const std::string&);
    int createConnection(const std::string&, int);
    void connectionEvent(int, uint32_t);
public:
    TCPClient();
    void run(int, int, const std::string&, int);
};


#endif //TCP_CALCULATOR_TCPCLIENT_H
