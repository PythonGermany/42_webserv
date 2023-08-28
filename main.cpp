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

std::string ntos(struct in_addr &addr)
{
    std::stringstream ss;
    unsigned char *p = reinterpret_cast<unsigned char *>(&addr.s_addr);
    for (size_t i = 0; i < 4; ++i)
    {
        ss << static_cast<int>(p[i]);
        if (i + 1 != 4)
            ss << '.';
    }
    return ss.str();
}

int main()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
        throw std::runtime_error(std::strerror(errno));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(8080);
    addr.sin_family = AF_INET;
    if (bind(socketfd, (struct sockaddr*)&addr, sizeof(addr)))
        throw std::runtime_error(std::strerror(errno));
    if (listen(socketfd, SOMAXCONN))
        throw std::runtime_error(std::strerror(errno));
    std::vector<struct pollfd> fd;
    struct pollfd newfd;
    newfd.fd = socketfd;
    newfd.events = POLLIN;
    fd.push_back(newfd);
    while (true)
    {
        int ready = poll(&fd[0], fd.size(), -1);

        if (ready > 0)
        {
            if (fd[0].revents & POLLIN)
            {
                struct sockaddr_in clientAddress;
                socklen_t clientAddressLen = sizeof(clientAddress);
                int clientSocket = accept(fd[0].fd, (sockaddr *)&clientAddress, &clientAddressLen);
                if (clientSocket > 0)
                {
                    std::cout << ntos(clientAddress.sin_addr) << std::endl;
                    close(clientSocket);
                    close(fd[0].fd);
                    return 0;
                }
            }
        }
    }
}
