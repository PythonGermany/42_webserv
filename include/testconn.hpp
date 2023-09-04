#ifndef TESTCONN_HPP
# define TESTCONN_HPP

# include <iostream>
# include "Connection.hpp"

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

#endif //TESTCONN_HPP
