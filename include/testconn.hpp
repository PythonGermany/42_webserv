#ifndef TESTCONN_HPP
# define TESTCONN_HPP

# include <iostream>
# include "Connection.hpp"

/**
 * implements recv() and can use:
 * send()
 * _msgsize
 * _msgdelimiter
 * _addr: address of the client (READ ONLY)
 * _host: address of the host socket (READ ONLY)
 * 
 * TODO:
 * no functionality to close connection
*/
class testconn : public Connection
{
public:
    testconn(Address const &addr, Address const &host) : Connection(addr, host) {}
private:
    void recv(std::string msg)
    {
        (void)_msgsize;
        (void)_msgdelimiter;
        (void)_addr;
        (void)_host;
        std::cout << "########### BEGIN ##########" << std::endl;
        std::cout << msg;
        std::cout << "###########  END  ##########" << std::endl;
        send("Welcome to this server\n");
    };
};

#endif //TESTCONN_HPP
