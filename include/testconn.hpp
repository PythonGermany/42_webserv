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
        std::cout << "$$$$$$$$$ BEGIN HEAD $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END HEAD $$$$$$$$$$$" << std::endl;
        send("Welcome! you send a HEAD\n");
        std::vector<std::string> arg;
        std::vector<std::string> env;
        env.push_back("CGI_TEST=1234");
        env.push_back("CGI_TEST2=abc");
        runCGI("/usr/bin/env", arg, env);
    }
    void OnBodyRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN BODY $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END BODY $$$$$$$$$$$" << std::endl;
        send("Welcome! you send a BODY\n");
    }
    void OnCgiRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN CGI $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END CGI $$$$$$$$$$$" << std::endl;
    }
};

#endif //TESTCONN_HPP
