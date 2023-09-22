#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <csignal>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <cerrno>
#include <arpa/inet.h>

#define NUM_OF_SOCKETS 10

void executeWebserv()
{
    char const *arg[] = {"../../webserv", "test.conf", NULL};
    execve("../../webserv", (char *const *)arg, NULL);
}

int main()
{
    int webservPid = fork();

    if (webservPid == 0)
    {
        close(1);
        close(2);
        executeWebserv();
        std::exit(EXIT_FAILURE);
    }
    sleep(1);
    std::vector<int> sockets;
    for (size_t i = 0; i < NUM_OF_SOCKETS; ++i)
    {
        struct addrinfo hints = {}, *result = NULL;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo("127.0.0.1", "8080", &hints, &result) != 0 || result == NULL)
        {
            std::cerr << "error: getaddrinfo()" << std::endl;
            kill(webservPid, SIGKILL);
            return 1;
        }
        int fd = socket(result->ai_family, SOCK_STREAM, 0);
        if (fd == -1)
        {
            std::cerr << "error: socket()" << std::endl;
            kill(webservPid, SIGKILL);
            return 1;
        }
        if (connect(fd, result->ai_addr, result->ai_addrlen) == -1)
        {
            std::cerr << "error: connect(): " << std::strerror(errno) << std::endl;
            kill(webservPid, SIGKILL);
            return 1;
        }
        std::cout << "established connection with fd: " << fd << std::endl;
        freeaddrinfo(result);
        sockets.push_back(fd);
    }
    for (size_t i = 0; i < NUM_OF_SOCKETS; ++i)
    {
        if (i % 2)
            continue;
        close(sockets[i]);
        std::cout << "close connection with fd: " << sockets[i] << std::endl;
    }
    for (size_t i = 0; i < NUM_OF_SOCKETS; ++i)
    {
        if (!(i % 2))
            continue;
        close(sockets[i]);
        std::cout << "close connection with fd: " << sockets[i] << std::endl;
    }
    kill(webservPid, SIGINT);
    int status;
    if (waitpid(webservPid, &status, 0) == -1)
    {
        kill(webservPid, SIGKILL);
        return 1;
    }
    if (WEXITSTATUS(status))
        std::cout << "ERROR" << std::endl;
    else
        std::cout << "SUCCESS" << std::endl;
    return WEXITSTATUS(status);
}
