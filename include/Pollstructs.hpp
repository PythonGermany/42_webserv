#ifndef POLLSTRUCTS_HPP
# define POLLSTRUCTS_HPP

# include <vector>
# include <poll.h>
# include <unistd.h>
# include "testconn.hpp"
# include "ListenSocket.hpp"

class Pollstructs
{
public:
    typedef struct pollfd value_type;
    typedef testconn conn_type;
    typedef std::vector<value_type>::iterator iterator;
    Pollstructs();
    ~Pollstructs();
    void add(ListenSocket const &sock);
    void poll();
private:
    std::vector<value_type>  _vec;
    std::vector<ListenSocket> listenSockets;
    std::vector<conn_type> connections;

    void updateLisetnSockets();
    void updateConnection();
};

#endif //POLLSTRUCTS_HPP
