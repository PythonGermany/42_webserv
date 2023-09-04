#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include "Address.hpp"
# include <string>
# include <vector>
# include <poll.h>
# include <stdexcept>
# include <cerrno>
# include <cstring>

# define BUFFER_SIZE 65536

class Connection
{
public:
    Address _addr;
    Address _host;
    Connection(Address const &addr, Address const &host);
    virtual ~Connection() {}
    virtual void recv(std::string msg) = 0;
    void send(std::string msg);
    void pollout(std::vector<struct pollfd>::iterator pollfd);
    void pollin(std::vector<struct pollfd>::iterator pollfd);
protected:
    std::string::size_type _maxreadsize;
    std::string::size_type _msgsize;
    std::string _msgdelimeter;
private:
    std::string _writebuffer;
    std::string _readbuffer;
};

#endif //CONNECTION_HPP
