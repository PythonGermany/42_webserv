
#ifndef LISTENSOCKET_HPP
# define LISTENSOCKET_HPP

# include "IFileDescriptor.hpp"
# include "Address.hpp"

# include <string>

class ListenSocket : public IFileDescriptor
{
public:
    Address _addr;
    ListenSocket();
    ListenSocket(std::string const &addr, std::string const &port, int backlog = SOMAXCONN);
    ListenSocket(ListenSocket const &other);
    ~ListenSocket();
    ListenSocket &operator=(ListenSocket const &other);
    void pollout(struct pollfd &pollfd);
    void pollin(struct pollfd &pollfd);
};

#endif //LISTENSOCKET_HPP
