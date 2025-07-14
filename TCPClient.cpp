//
// Created by urus on 13.07.25.
//

#include "TCPClient.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <algorithm>

TCPClient::TCPClient() : epollFD(-1), gen(std::chrono::steady_clock::now().time_since_epoch().count()) {}

void TCPClient::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

std::vector<std::string> TCPClient::splitExpression(const std::string& expression) {
    std::vector<std::string> fragments;
    std::uniform_int_distribution<> fragmentsDistrib(2, 5);
    int countOfFragments = fragmentsDistrib(gen);

    if (countOfFragments >= expression.length()) {
        for (char c : expression) {
            fragments.push_back(std::string(1, c));
        }
        return fragments;
    }

    std::vector<int> splitPoints;
    std::uniform_int_distribution<> splitDistrib(1, expression.length() - 1);

    for (int i = 0; i < countOfFragments - 1; i++) {
        splitPoints.push_back(splitDistrib(gen));
    }
    splitPoints.push_back(0);
    splitPoints.push_back(expression.length());
    std::sort(splitPoints.begin(), splitPoints.end());
    splitPoints.erase(std::unique(splitPoints.begin(), splitPoints.end()), splitPoints.end());

    for (size_t i = 0; i < splitPoints.size() - 1; i++) {
        fragments.push_back(expression.substr(splitPoints[i], splitPoints[i + 1] - splitPoints[i]));
    }

    return fragments;
}

int TCPClient::createConnection(const std::string& serverAddr, int serverPort) {
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD < 0) {
        std::cerr << "Failed to create socket\n";
        return -1;
    }

    int flag = 1;
    setsockopt(socketFD, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    setNonBlocking(socketFD);

    struct sockaddr_in server = {};
    server.sin_family = AF_INET;
    server.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverAddr.c_str(), &server.sin_addr) <= 0) {
        std::cerr << "Invalid address: " << serverAddr << '\n';
        close(socketFD);
        return -1;
    }

    int ret = connect(socketFD, (struct sockaddr*)&server, sizeof(server));
    if (ret < 0 && errno != EINPROGRESS) {
        std::cerr << "Failed to connect\n";
        close(socketFD);
        return -1;
    }

    return socketFD;
}

void TCPClient::connectionEvent(int fd, uint32_t events) {
    ConnectionState& state = connections[fd];

    if (events & EPOLLOUT) {
        if (!state.fragments.empty()) {
            const std::string& fragment = state.fragments.front();
            ssize_t sent = send(fd, fragment.c_str(), fragment.length(), MSG_NOSIGNAL);

            if (sent > 0) {
                if (sent == fragment.length()) {
                    state.fragments.pop();
                } else {
                    state.fragments.front() = fragment.substr(sent);
                }
            }

            if (state.fragments.empty()) {
                state.isSendingComplete = true;
                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.fd = fd;
                epoll_ctl(epollFD, EPOLL_CTL_MOD, fd, &event);
            }
        }
    }

    if (events & EPOLLIN) {
        char buffer[BUFFER_SIZE];
        ssize_t readedBytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

        if (readedBytes > 0) {
            buffer[readedBytes] = '\0';
            state.buffer += buffer;

            size_t pos = state.buffer.find(' ');
            if (pos != std::string::npos) {
                std::string response = state.buffer.substr(0, pos);

                if (response == "ERROR") {
                    std::cerr << "Server returned ERROR for expression: "
                              << state.expression << '\n';
                } else {
                    try {
                        double serverResult = std::stod(response);
                        double diff = std::abs(serverResult - state.expected);

                        if (diff > 1e-6) {
                            std::cerr << "Wrong result! Expression: " << state.expression
                                      << " Server result: " << serverResult
                                      << " Correct result: " << state.expected
                                      << '\n';
                        } else {
                            std::cout << "OK: " << state.expression.substr(0, state.expression.length()-1)
                                      << " = " << serverResult << '\n';
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Failed to parse server response: " << response << '\n';
                    }
                }

                epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                connections.erase(fd);
            }
        } else {
            if (readedBytes == 0) {
                std::cout << "Server closed connection for fd " << fd << "\n";
            }
            epoll_ctl(epollFD, EPOLL_CTL_DEL, fd, nullptr);
            close(fd);
            connections.erase(fd);
        }
    }
}

void TCPClient::run(int n, int countOfConnections, const std::string& serverAddr, int serverPort) {
    epollFD = epoll_create1(0);
    if (epollFD < 0) {
        throw std::runtime_error("Failed to create epoll");
    }

    for (int i = 0; i < countOfConnections; i++) {
        int fd = createConnection(serverAddr, serverPort);
        if (fd < 0) continue;

        auto [expression, result] = generator.generate(n);

        ConnectionState state;
        state.fd = fd;
        state.expression = expression;
        state.expected = result;
        state.isSendingComplete = false;

        auto fragments = splitExpression(expression);
        for (const auto& fragment : fragments) {
            state.fragments.push(fragment);
        }

        connections[fd] = state;

        struct epoll_event event;
        event.events = EPOLLOUT;
        event.data.fd = fd;
        epoll_ctl(epollFD, EPOLL_CTL_ADD, fd, &event);
    }

    struct epoll_event events[BUFFER_SIZE];

    while (!connections.empty()) {
        int nfds = epoll_wait(epollFD, events, BUFFER_SIZE, 1000);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("epoll_wait failed");
        }

        for (int i = 0; i < nfds; i++) {
            connectionEvent(events[i].data.fd, events[i].events);
        }
    }

    close(epollFD);
}
