
#ifndef LISTENSOCKET_HPP
# define LISTENSOCKET_HPP

# include "IFileDescriptor.hpp"
# include "Address.hpp"

# include <string>

class ListenSocket : public IFileDescriptor
{
public:
    ListenSocket(std::string const &addr, std::string const &port, int backlog = SOMAXCONN);
    ListenSocket(ListenSocket const &other);
    ~ListenSocket();
    ListenSocket &operator=(ListenSocket const &other);
    void onPollEvent(struct pollfd &pollfd);
private:
    Address _addr;

    ListenSocket();
};

#endif //LISTENSOCKET_HPP
