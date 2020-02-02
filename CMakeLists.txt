cmake_minimum_required(VERSION 3.5)

project(CppSockets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CppSockets main.cpp
                          TCPSocket.cpp
                          TCPSocket.hpp
                          IpAddress.cpp
                          IpAddress.hpp)

add_executable(CppSockets2 main2.cpp
                           TCPSocket.cpp
                           TCPSocket.hpp
                           IpAddress.cpp
                           IpAddress.hpp)
