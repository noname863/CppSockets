#include <iostream>
#include "TCPSocket.hpp"
#include <cstring>
#include <cassert>

int main()
{
    TCPSocket socket;
    socket.setBlocking(true);
    std::cout << "connecting" << std::endl;
    assert(socket.connect({127, 0, 0, 1}, 11115) == 0);
    std::cout << "connected" << std::endl;
    unsigned char size;
    char * message = new char[256];
    std::cin.getline(message, 256);
    size = std::strlen(message);
    socket.send(&size, 1);
    socket.send(message, size);
    std::cout << "message sended" << std::endl;
    std::cout << "waiting for response" << std::endl;
    socket.receive(&size, 1);
    socket.receive(message, size);
    message[size] = 0;
    std::cout << "response: " << std::string(message) << std::endl;
    return 0;
}
