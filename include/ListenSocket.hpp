
#ifndef LISTENSOCKET_HPP
# define LISTENSOCKET_HPP
# include <fcntl.h>
# include "Address.hpp"
# include <stdexcept>
# include <cerrno>
# include <cstring>
# include <string>

class ListenSocket
{
public:
    int fd;
    Address _addr;
    ListenSocket();
    ListenSocket(std::string const &addr, std::string const &port, int backlog = SOMAXCONN);
    ListenSocket(ListenSocket const &other);
    ~ListenSocket();
    ListenSocket &operator=(ListenSocket const &other);
    int accept(Address &addr) const;
};

#endif //LISTENSOCKET_HPP
