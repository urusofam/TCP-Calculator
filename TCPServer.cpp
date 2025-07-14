//
// Created by urus on 12.07.25.
//

#include "TCPServer.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>

TCPServer::TCPServer(int port_) : port(port_), serverFD(-1), epollFD(-1) {}

TCPServer::~TCPServer() {
    if (epollFD >= 0) close(epollFD);
    if (serverFD >= 0) close(serverFD);
}

void TCPServer::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void TCPServer::readClientData(int clientFD) {
    char buffer[BUFFER_SIZE];
    ssize_t readedBytes = recv(clientFD, buffer, BUFFER_SIZE - 1, 0);

    if (readedBytes <= 0) {
        if (readedBytes == 0) {
            std::cout << "Client " << clientFD << " disconnected\n";
        }
        epoll_ctl(epollFD, EPOLL_CTL_DEL, clientFD, nullptr);
        close(clientFD);
        clientBuffers.erase(clientFD);
        return;
    }

    buffer[readedBytes] = '\0';
    clientBuffers[clientFD] += buffer;
    std::string& clientBuffer = clientBuffers[clientFD];

    size_t position;
    while((position = clientBuffer.find(' ')) != std::string::npos) {
        std::string expression = clientBuffer.substr(0, position);
        clientBuffer.erase(0, position + 1);

        if(!expression.empty()) {
            sendAnswer(clientFD, expression);
        }
    }
}

void TCPServer::sendAnswer(int clientFD, const std::string& expression) {
    try {
        double result = calculator.calculate(expression);
        std::string response = std::to_string(result) + ' ';
        send(clientFD, response.c_str(), response.length(), MSG_NOSIGNAL);
    } catch (const std::exception& error) {
        std::string response = "ERROR ";
        send(clientFD, response.c_str(), response.length(), MSG_NOSIGNAL);
    }
}

void TCPServer::run() {
    serverFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFD < 0)  {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(serverFD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }

    setNonBlocking(serverFD);

    if (listen(serverFD, SOMAXCONN) < 0) {
        throw std::runtime_error("Failed to listen socket");
    }

    epollFD = epoll_create1(0);
    if (epollFD < 0) {
        throw std::runtime_error("Failed to create epoll");
    }

    struct epoll_event event = {};
    event.events = EPOLLIN;
    event.data.fd = serverFD;
    if (epoll_ctl(epollFD, EPOLL_CTL_ADD, serverFD, &event) < 0) {
        throw std::runtime_error("Failed to add server socket to epoll");
    }
    std::cout << "Server started on port " << port << '\n';

    struct epoll_event events[BUFFER_SIZE];

    while (true) {
        int eventsCount = epoll_wait(epollFD, events, BUFFER_SIZE, -1);
        if (eventsCount < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("Failed to wait for events");
        }

        for (int i = 0; i < eventsCount; i++) {
            if (events[i].data.fd == serverFD) {
                struct sockaddr_in clientAddr = {};
                socklen_t clientAddrLen = sizeof(clientAddr);

                int clientFD = accept(serverFD, (struct sockaddr*)&clientAddr, &clientAddrLen);
                if (clientFD < 0) {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        std::cerr << "Failed to accept connection\n";
                    }
                    continue;
                }

                setNonBlocking(clientFD);

                struct epoll_event clientEvent;
                clientEvent.events = EPOLLIN;
                clientEvent.data.fd = clientFD;
                if (epoll_ctl(epollFD, EPOLL_CTL_ADD, clientFD, &clientEvent) < 0) {
                    std::cerr << "Failed to add client to epoll\n";
                    close(clientFD);
                    continue;
                }
                std::cout << "New client connected: " << clientFD << '\n';
            } else {
                readClientData(events[i].data.fd);
            }
        }
    }
}