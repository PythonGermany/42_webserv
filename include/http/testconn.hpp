#ifndef TESTCONN_HPP
# define TESTCONN_HPP

# include "../poll/AConnection.hpp"

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
        if (msg[0] != 'c')
        {
            send("invalid. start with 'c'\n");
            closeConnection();
        }
        if (msg != "cgi\r\n\r\n")
            return;
        std::vector<std::string> args;
        args.push_back("Hello from Cgi program");
        std::vector<std::string> env;
        cgiSend("this is from stdin\n");
        runCGI("./cgi/cgi", args, env);
        send("You requested a cgi\n");
    }
    void OnBodyRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN BODY $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END BODY $$$$$$$$$$$" << std::endl;
    }
    void OnCgiRecv(std::string msg)
    {
        std::cout << "$$$$$$$$$ BEGIN CGI $$$$$$$$$$ >";
        std::cout << msg;
        std::cout << "< $$$$$$$$$$ END CGI $$$$$$$$$$$" << std::endl;
        send(msg);
    }
    void OnCgiTimeout()
    {
        send("cgi timeout\n");
    }
};

#endif //TESTCONN_HPP
