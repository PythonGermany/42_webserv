#ifndef TESTCONN_HPP
# define TESTCONN_HPP

# include "AConnection.hpp"

# include <iostream>

class testconn : public AConnection
{
public:
    testconn(Address const &client, Address const &host)
    {
        this->client = client;
        this->host = host;
        this->msgdelimiter = "\r\n\r\n";
        this->msgsizelimit = 10000;
        this->msgsize = 100;
    }
    void OnHeadRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN $$$$$$$$$$" << std::endl;
        std::cout << msg << std::endl;
        std::cout << "$$$$$$$$$$ END $$$$$$$$$$$" << std::endl;
        send("Welcome!\n");
    }
    void OnBodyRecv(std::string) {}
    void OnCgiRecv(std::string) {}
};

#endif //TESTCONN_HPP
