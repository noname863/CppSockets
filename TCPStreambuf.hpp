#ifndef TCPSTREAMBUF_HPP
#define TCPSTREAMBUF_HPP
#include <streambuf>

class TCPStreambuf : std::streambuf
{
    int socket;
public:
    TCPStreambuf(int socket);
    virtual int_type uflow() override;
    virtual std::streamsize xsgetn(char * str, std::streamsize n) override;
    void processError();
};

#endif // TCPSTREAMBUF_HPP
