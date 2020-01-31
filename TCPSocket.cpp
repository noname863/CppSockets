#include "TCPSocket.hpp"
// TODO: add windows headers
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

sockaddr_in TCPSocket::createAddress(uint32_t address, uint16_t port) {
    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = address;
    addr.sin_family      = AF_INET;
    addr.sin_port        = port;

    return addr;
}

void TCPSocket::create() {
    nagleEnabled = true;
    blocking = true;
    closed = true;
    recreate();
}

void TCPSocket::recreate() {
    if (this->closed == true) {
        this->socket = ::socket(AF_INET, SOCK_STREAM, 0);
        this->setNagle(nagleEnabled);
        this->closed = false;
    }
}

int TCPSocket::getError() {
    return errno;
}

TCPSocket::TCPSocket() {
    create();
}

int TCPSocket::getSocketHandle() {
    return this->socket;
}

int TCPSocket::connect(const IpAddress & address, uint16_t port)
{
    // create socket if it was closed
    recreate();
    
    sockaddr_in sockApiAddr = createAddress(address, htons(port));
    
    if (!::connect(this->socket, reinterpret_cast<sockaddr *>(&sockApiAddr), sizeof(sockaddr_in))) {
        return getError();
    } else {
        if (blocking) {
            waitWriteReady();
            // check if connected
        }
        connected = true;        
    }
    return 0;
}

int TCPSocket::connect(const IpAddress & address, uint16_t port, std::chrono::microseconds & mc) {
    recreate();
    
    sockaddr_in sockApiAddr = createAddress(address, htons(port));
    
    if (!::connect(this->socket, reinterpret_cast<sockaddr *>(&sockApiAddr), sizeof(sockaddr_in))) {
        return getError();
    } else {
        if (blocking) {
            waitWriteReady(mc);
            // check if connected
            if (mc == std::chrono::microseconds(0)) {
                return 0;
            }
        }
        connected = true;
    }
    return 0;
}

int TCPSocket::close() {
    ::close(this->socket);
}

bool TCPSocket::isConnected() {
    return connected;
}
