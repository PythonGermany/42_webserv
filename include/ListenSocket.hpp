
#ifndef LISTENSOCKET_HPP
# define LISTENSOCKET_HPP
# include <fcntl.h>
# include "Address.hpp"
# include <stdexcept>
# include <cerrno>
# include <cstring>

/**
 * TODO:
 * @param max_queue_size default should be SOMAXCONN
*/
class ListenSocket
{
public:
    int fd;
    Address _addr;
    ListenSocket(char const *ip, char const *port);
    ListenSocket(ListenSocket const &other);
    int accept(Address &addr) const;
};

#endif //LISTENSOCKET_HPP
