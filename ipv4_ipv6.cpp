#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <exception>
#include <netinet/in.h>
#include <vector>
#include <signal.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sstream>
#include <cstring>
#include <cerrno>

#include "Address.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3)
        return 1;
    Address addr(argv[1]);
    addr.port(std::atoi(argv[2]));
    std::cout << addr.family() << std::endl;
    int socketfd = socket(addr.family(), SOCK_STREAM, 0);
    if (socketfd < 0)
        throw std::runtime_error(std::string("socket(): ") + std::strerror(errno));

    int reuse = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        throw std::runtime_error(std::string("setsockopt(): ") + std::strerror(errno));

    if (bind(socketfd, addr.data(), addr.size()))
        throw std::runtime_error(std::string("bind(): ") + std::strerror(errno));
    if (listen(socketfd, SOMAXCONN))
        throw std::runtime_error(std::string("listen(): ") + std::strerror(errno));
    Address addrclient;
    socklen_t  len = sizeof(sockaddr_in6);
    int client = accept(socketfd, addrclient.data(), &len);
    if (addrclient.size() != len)
        throw std::runtime_error(std::string("accept(): ") + std::strerror(errno));
    std::cout << addrclient << std::endl;
    std::cout << addrclient.family() << std::endl;
    close(client);
    close(socketfd);
}
