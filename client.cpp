//
// Created by urus on 13.07.25.
//

#include <iostream>
#include "TCPClient.h"

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <n> <connections> <server_addr> <server_port>\n";
        return 1;
    }
    int n = std::stoi(argv[1]);
    int connections = std::stoi(argv[2]);
    std::string server_addr = argv[3];
    int server_port = std::stoi(argv[4]);

    try {
        TCPClient client;
        client.run(n, connections, server_addr, server_port);
    } catch (const std::exception& e) {
        std::cerr << "Client error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}