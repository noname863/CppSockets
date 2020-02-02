#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include "TCPSocket.hpp"
#include "IpAddress.hpp"
#include <cstring>
#include <assert.h>

int main() {
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd != -1);
    sockaddr_in localAddr;
    std::memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = IpAddress(127, 0, 0, 1);
    localAddr.sin_port = htons(11115);
    assert(::bind(sockfd, reinterpret_cast<sockaddr *>(&localAddr), sizeof(sockaddr_in)) != -1);    
    assert(::listen(sockfd, 1) != -1);
    std::cout << "waiting for connection" << std::endl;
    TCPSocket socket(::accept(sockfd, nullptr, nullptr));
    ::close(sockfd);
    std::cout << "connected" << std::endl;
    socket.setBlocking(true);
    std::cout << "waiting for message" << std::endl;
    unsigned char size;
    socket.receive(&size, 1);
    char * message = new char[size + 1];
    socket.receive(message, size);
    message[size] = 0;
    std::cout << "message: " << std::string(message) << std::endl;
    std::cout << "sending responce" << std::endl;
    delete[] message;
    message = new char[256];
    std::cin.getline(message, 256);
    size = static_cast<unsigned char>(std::strlen(message));
    socket.send(&size, 1);
    socket.send(message, size);
    std::cout << "response sended" << std::endl;
    delete[] message;
    socket.close();
}
