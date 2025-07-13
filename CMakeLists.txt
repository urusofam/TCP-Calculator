cmake_minimum_required(VERSION 3.22)
project(TCP-Calculator CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(serverLib
        Calculator.cpp
        Calculator.h
        TCPServer.cpp
        TCPServer.h
)

add_executable(server server.cpp)

target_link_libraries(server serverLib)