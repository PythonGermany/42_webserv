#include "../include/ListenSocket.hpp"
#include "../include/Poll.hpp"
#include "../include/testconn.hpp"

#include <fcntl.h>

ListenSocket::ListenSocket()
{
}

ListenSocket::ListenSocket(std::string const &addr, std::string const &port, int backlog)
    :   _addr(addr, port)
{
    struct pollfd pollfd;

    pollfd.fd = socket(_addr.family(), SOCK_STREAM, 0);
    if (pollfd.fd < 0)
        throw std::runtime_error(std::string("ListenSocket(): socket(): ") + std::strerror(errno));
    
    int flags = fcntl(pollfd.fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error(std::string("ListenSocket(): fcntl(): ") + std::strerror(errno));
    if (fcntl(pollfd.fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error(std::string("ListenSocket(): fcntl(): ") + std::strerror(errno));

    int reuse = 1;
    if (setsockopt(pollfd.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        throw std::runtime_error(std::string("ListenSocket(): setsockopt(): ") + std::strerror(errno));

    if (bind(pollfd.fd, _addr.data(), _addr.size()))
        throw std::runtime_error(std::string("ListenSocket(): bind(): ") + std::strerror(errno));

    if (listen(pollfd.fd, backlog))
        throw std::runtime_error(std::string("ListenSocket(): listen(): ") + std::strerror(errno));

    pollfd.events = POLLIN;
    pollfd.revents = 0;
    Poll::add(pollfd);
    std::cout << "listen: " << _addr << std::endl;
}

ListenSocket::ListenSocket(ListenSocket const &other) : _addr(other._addr)
{
}

ListenSocket::~ListenSocket()
{
    std::cout << "stop listening: " << _addr << std::endl;
}

ListenSocket &ListenSocket::operator=(ListenSocket const &other)
{
    if (this != &other)
    {
        _addr = other._addr;
    }
    return *this;
}

void ListenSocket::onPollOut(struct pollfd &)
{
}

void ListenSocket::onPollIn(struct pollfd &pollfd)
{
    struct pollfd newPollfd;
    socklen_t len = sizeof(sockaddr_in6);
    Address client;

    pollfd.revents &= ~POLLIN;
    newPollfd.fd = ::accept(pollfd.fd, client.data(), &len);
    if (newPollfd.fd == -1)
        throw std::runtime_error(std::string("ListenSocket::onPollIn(): ") + std::strerror(errno));
    int flags = fcntl(newPollfd.fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error(std::string("ListenSocket::onPollIn(): ") + std::strerror(errno));
    std::cout << (flags & O_RDWR) << std::endl;
    client.size(len);
    newPollfd.events = POLLIN;
    newPollfd.revents = 0;
    Poll::add(new testconn(client, _addr));
    Poll::add(newPollfd);
}

void ListenSocket::onNoPollIn(struct pollfd &)
{
}
