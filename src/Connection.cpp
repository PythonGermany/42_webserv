#include "../include/Connection.hpp"

Connection::Connection(Address const &addr, Address const &host)
    :   _addr(addr),
        _host(host),
        _maxreadsize(8192),
        _msgsize(std::string::npos),
        _msgdelimeter("\r\n\r\n")
{}

void Connection::send(std::string msg)
{
    _writebuffer += msg;
}

void Connection::pollout(std::vector<struct pollfd>::iterator pollfd)
{
    size_t lentosend;
    ssize_t lensent;

    pollfd->revents &= ~POLLOUT;
    if (_writebuffer.size() > BUFFER_SIZE)
        lentosend = BUFFER_SIZE;
    else
        lentosend = _writebuffer.size();
    lensent = ::send(pollfd->fd, _writebuffer.data(), lentosend, 0);
    if (lensent == -1)
        throw std::runtime_error(std::string("Connection::pollout(): ") + std::strerror(errno));
    _writebuffer.erase(0, lensent);
    if (_writebuffer.empty())
    {
        pollfd->events &= ~POLLOUT;
    }
}

void Connection::pollin(std::vector<struct pollfd>::iterator pollfd)
{
    pollfd->revents &= ~POLLIN;
    char tmpbuffer[BUFFER_SIZE];

    ssize_t msglen = ::recv(pollfd->fd, tmpbuffer, BUFFER_SIZE, 0);
    if (msglen < 0)
        throw std::runtime_error(std::string("Connection::pollin(): ") + std::strerror(errno));
    if (msglen == 0)
        pollfd->events &= ~POLLIN;
    _readbuffer += std::string(tmpbuffer, tmpbuffer + msglen);
    if (_readbuffer.size() > _maxreadsize)
    {
        pollfd->events = 0;
        pollfd->revents = 0;
        return;
    }
    std::string::size_type pos = _readbuffer.find(_msgdelimeter);
    while (pos != std::string::npos)
    {
        pos += _msgdelimeter.size();
        recv(_readbuffer.substr(0, pos));
        _readbuffer.erase(0, pos);
        pos = _readbuffer.find(_msgdelimeter);
    }
    while (_readbuffer.size() > _msgsize)
    {
        recv(_readbuffer.substr(0, _msgsize));
        _readbuffer.erase(0, _msgsize);
    }
    if (!_writebuffer.empty())
    {
        pollfd->events |= POLLOUT;
    }
}
