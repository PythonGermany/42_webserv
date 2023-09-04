#include "../include/Pollstructs.hpp"

Pollstructs::Pollstructs()
{}

Pollstructs::~Pollstructs()
{
    for (iterator it = _vec.begin(); it != _vec.end(); ++it)
    {
        close(it->fd);
    }
}

void Pollstructs::add(ListenSocket const &sock)
{
    struct pollfd tmp;

    tmp.events = POLLIN;
    tmp.fd = sock.fd;
    _vec.push_back(tmp);
    listenSockets.push_back(sock);
}

void Pollstructs::poll()
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

void Pollstructs::updateLisetnSockets()
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
void Pollstructs::updateConnection()
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
