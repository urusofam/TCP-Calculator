//
// Created by urus on 12.07.25.
//

#ifndef TCP_CALCULATOR_TCPSERVER_H
#define TCP_CALCULATOR_TCPSERVER_H

#include "Calculator.h"
#include <unordered_map>

class TCPServer {
    int serverFD;
    int epollFD;
    int port;
    Calculator calculator;
    static const int BUFFER_SIZE = 1024;
    std::unordered_map<int, std::string> clientBuffers;

    void setNonBlocking(int);
    void readClientData(int);
    void sendAnswer(int, const std::string&);
public:
    explicit TCPServer(int);
    ~TCPServer();
    void run();
};


#endif //TCP_CALCULATOR_TCPSERVER_H
