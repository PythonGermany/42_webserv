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
        this->msgsize = 10000;
        std::cout << "create: " << client << std::endl;
    }
    ~testconn()
    {
        std::cout << "delete: " << client << std::endl;
    }
    void OnHeadRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN HEAD $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END HEAD $$$$$$$$$$$" << std::endl;
        send("HTTP/1.1 200 OK\r\n");
        send("Content-Length: 20\r\n");
        send("Content-Type: text/html\r\n");
        send("\r\n");
        send("<h1>Hello World</h1>");
    }
    void OnBodyRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN BODY $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END BODY $$$$$$$$$$$" << std::endl;
        // send("Welcome! you send a BODY\n");
    }
    void OnCgiRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN CGI $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END CGI $$$$$$$$$$$" << std::endl;
    }
};

#endif //TESTCONN_HPP
