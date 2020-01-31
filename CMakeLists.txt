cmake_minimum_required(VERSION 3.5)

project(CppSockets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(CppSockets main.cpp
                          TCPStreambuf.cpp
                          TCPStreambuf.hpp
                          TCPSocket.cpp
                          TCPSocket.hpp
                          IpAddress.cpp
                          IpAddress.hpp)
