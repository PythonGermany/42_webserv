#include "../include/ListenSocket.hpp"

ListenSocket::ListenSocket(char const *ip, char const *port)
    :   _addr(ip)
{
    _addr.port(std::atoi(port));
    fd = socket(_addr.family(), SOCK_STREAM, 0);
    if (fd < 0)
        throw std::runtime_error(std::string("ListenSocket(): socket(): ") + std::strerror(errno));
    
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error(std::string("ListenSocket(): fcntl(): ") + std::strerror(errno));
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw std::runtime_error(std::string("ListenSocket(): fcntl(): ") + std::strerror(errno));

    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        throw std::runtime_error(std::string("ListenSocket(): setsockopt(): ") + std::strerror(errno));

    if (bind(fd, _addr.data(), _addr.size()))
        throw std::runtime_error(std::string("ListenSocket(): bind(): ") + std::strerror(errno));

    if (listen(fd, SOMAXCONN))
        throw std::runtime_error(std::string("ListenSocket(): listen(): ") + std::strerror(errno));
}

ListenSocket::ListenSocket(ListenSocket const &other)
    :   fd(other.fd),
        _addr(other._addr)
{}

int ListenSocket::accept(Address &addr) const
{
    int _newfd;
    socklen_t len = sizeof(sockaddr_in6);
    _newfd = ::accept(fd, addr.data(), &len);
    if (_newfd < 0)
        throw std::runtime_error(std::string("ListenSocket::accept(): ") + std::strerror(errno));
    return _newfd;
}
