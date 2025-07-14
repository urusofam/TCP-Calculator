cmake_minimum_required(VERSION 3.22)
project(TCP-Calculator CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(serverLib
        TCPServer.cpp
        TCPServer.h
)

add_library(calculatorLib
        Calculator.cpp
        Calculator.h
)

add_library(clientLib
        TCPClient.cpp
        TCPClient.h
        Generator.h
        Generator.cpp
)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(server serverLib calculatorLib)
target_link_libraries(client clientLib calculatorLib)