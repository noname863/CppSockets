#include "TCPStreambuf.hpp"
#include <unistd.h>


TCPStreambuf::TCPStreambuf(int socket) {
    this->socket = socket;
}

TCPStreambuf::int_type TCPStreambuf::uflow() {
    
}

std::streamsize TCPStreambuf::xsgetn(char * str, std::streamsize n) {
    
}


