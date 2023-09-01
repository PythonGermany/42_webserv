#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <exception>
#include <netinet/in.h>
#include <vector>
#include <signal.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>
#include <cerrno>

#include "Address.hpp"

#include <fcntl.h>

#define BUFFER_SIZE 65536

/**
 * TODO:
 * @param max_queue_size default should be SOMAXCONN
*/
class ListenSocket
{
public:
    int fd;
    Address _addr;
    ListenSocket(char const *ip, char const *port)
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
    ListenSocket(ListenSocket const &other)
        :   fd(other.fd),
            _addr(other._addr)
    {}
    int accept(Address &addr) const
    {
        int _newfd;
        socklen_t len = sizeof(sockaddr_in6);
        _newfd = ::accept(fd, addr.data(), &len);
        if (_newfd < 0)
            throw std::runtime_error(std::string("ListenSocket::accept(): ") + std::strerror(errno));
        return _newfd;
    }
};

class Connection
{
public:
    Address _addr;
    Address _host;
    Connection(Address const &addr, Address const &host)
        :   _addr(addr),
            _host(host),
            _maxreadsize(8192),
            _msgsize(std::string::npos),
            _msgdelimeter("\r\n\r\n")
    {}
    virtual ~Connection() {}
    virtual void recv(std::string msg) = 0;
    void send(std::string msg)
    {
        _writebuffer += msg;
    }
    void pollout(std::vector<struct pollfd>::iterator pollfd)
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
    void pollin(std::vector<struct pollfd>::iterator pollfd)
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
protected:
    std::string::size_type _maxreadsize;
    std::string::size_type _msgsize;
    std::string _msgdelimeter;
private:
    std::string _writebuffer;
    std::string _readbuffer;
};

class testconn : public Connection
{
public:
    testconn(Address const &addr, Address const &host) : Connection(addr, host) {}
private:
    void recv(std::string msg)
    {
        std::cout << "########### BEGIN ##########" << std::endl;
        std::cout << msg;
        std::cout << "###########  END  ##########" << std::endl;
        send("Welcome to this server\n");
    };
};

class Pollstructs
{
public:
    typedef struct pollfd value_type;
    typedef testconn conn_type;
    typedef std::vector<value_type>::iterator iterator;
    Pollstructs() {}
    ~Pollstructs()
    {
        for (iterator it = begin(); it != end(); ++it)
        {
            close(it->fd);
        }
    }
    iterator begin()
    {
        return _vec.begin();
    }
    iterator end()
    {
        return _vec.end();
    }
    iterator erase(iterator it)
    {
        return _vec.erase(it);
    }
    void add(ListenSocket const &sock)
    {
        struct pollfd tmp;

        tmp.events = POLLIN;
        tmp.fd = sock.fd;
        _vec.push_back(tmp);
        listenSockets.push_back(sock);
    }
    void poll()
    {
        while (true)
        {
            int ready = ::poll(_vec.data(), _vec.size(), -1);
            if (ready == -1)
                throw std::runtime_error(std::string("Pollstructs::poll(): ") + std::strerror(errno));
            updateLisetnSockets();
            updateConnection();
        }
    }
private:
    std::vector<value_type>  _vec;
    std::vector<ListenSocket> listenSockets;
    std::vector<conn_type> connections;

    void updateLisetnSockets()
    {
        for (size_t i = 0; i < listenSockets.size(); ++i)
        {
            if (_vec.at(i).revents & POLLIN)
            {
                Address tmp;
                int fd = listenSockets.at(i).accept(tmp);
                struct pollfd pollfd;
                pollfd.events = POLLIN;
                pollfd.fd = fd;
                pollfd.revents = 0;
                _vec.push_back(pollfd);
                _vec.at(i).revents = (short)0;
                connections.push_back(conn_type(tmp, listenSockets.at(i)._addr));
                std::cout << tmp << " host: " << listenSockets.at(i)._addr << std::endl;
                std::cout << "size: " << _vec.size() << std::endl;
            }
        }
    }
    void updateConnection()
    {
        iterator it = _vec.begin() + listenSockets.size();
        std::vector<conn_type>::iterator connit = connections.begin();

        while (it != _vec.end())
        {
            if (it->revents & POLLIN)
                connit->pollin(it);
            if (it->revents & POLLOUT)
                connit->pollout(it);
            if ((it->events & POLLOUT) == 0 && (it->events & POLLIN) == 0)
            {
                std::cout << "close: " << connit->_addr << std::endl;
                close(it->fd);
                it = _vec.erase(it);
                connit = connections.erase(connit);
                std::cout << "size: " << _vec.size() << std::endl;
            }
            else
            {
                ++it;
                ++connit;
            }
        }
    }
};

int main()
{
    Pollstructs data;
    data.add(ListenSocket("127.0.0.1", "8080"));
    data.add(ListenSocket("::1", "8080"));
    data.add(ListenSocket("127.0.0.1", "9000"));
    data.poll();
}
