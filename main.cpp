#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#define MAX_CLIENTS 10

const std::string header =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 12\r\n"
    "Content-Type: text/plain\r\n"
    "Server: helloworld\r\n"
    "\r\n";

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " <port> [<port> ...]" << std::endl;
    return 1;
  }

  int server_dfs[argc - 1];
  struct sockaddr_in address;
  socklen_t address_len = sizeof(address);

  for (int i = 0; i < argc - 1; i++) {
    // Create socket
    server_dfs[i] = socket(AF_INET, SOCK_STREAM, 0);
    if (server_dfs[i] == -1) {
      std::cerr << "socket error: " << strerror(errno) << std::endl;
      return 1;
    }

    // Set socket options
    int port_reuse = 1;
    int setsockopt_result = setsockopt(server_dfs[i], SOL_SOCKET, SO_REUSEADDR,
                                       &port_reuse, sizeof(port_reuse));
    if (setsockopt_result < 0) {
      std::cerr << "setsockopt error: " << strerror(errno) << std::endl;
      return 1;
    }

    // Bind socket to address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(atoi(argv[i + 1]));
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    int bind_result = bind(server_dfs[i], (sockaddr*)&address, address_len);
    if (bind_result < 0) {
      std::cerr << "bind error: " << strerror(errno) << std::endl;
      return 1;
    }

    // Listen on socket
    int listen_result = listen(server_dfs[i], MAX_CLIENTS);
    if (listen_result < 0) {
      std::cerr << "listen error: " << strerror(errno) << std::endl;
      return 1;
    }

    std::cout << "Listening on port " << argv[i + 1] << std::endl;
  }

  int nfds = argc - 1;
  struct pollfd fds[MAX_CLIENTS + nfds];
  memset(fds, 0, sizeof(fds));

  for (int i = 0; i < nfds; i++) {
    fds[i].fd = server_dfs[i];
    fds[i].events = POLLIN;
  }

  while (true) {
    int poll_result = poll(fds, nfds, -1);

    if (poll_result < 0) {
      std::cerr << "poll error: " << strerror(errno) << std::endl;
      return 1;
    }

    for (int i = 0; i < nfds; i++) {
      if (fds[i].revents & POLLIN) {
        if (fds[i].fd == server_dfs[i]) {
          int client_df =
              accept(server_dfs[i], (sockaddr*)&address, &address_len);
          if (client_df < 0) {
            std::cerr << "accept error: " << strerror(errno) << std::endl;
            return 1;
          }

          fds[nfds].fd = client_df;
          fds[nfds].events = POLLIN;
          nfds++;
        } else {
          char buffer[1024] = {0};

          int read_result = read(fds[i].fd, buffer, sizeof(buffer));
          if (read_result < 0) {
            std::cerr << "read error: " << strerror(errno) << std::endl;
            return 1;
          } else {
            std::cout << buffer << std::endl;
          }

          std::string response = header + "Hello World!";
          int write_result =
              write(fds[i].fd, response.c_str(), response.size());
          if (write_result < 0) {
            std::cerr << "write error: " << strerror(errno) << std::endl;
            return 1;
          } else {
            std::cout << response << std::endl;
          }
          close(fds[i].fd);

          fds[i].fd = fds[nfds - 1].fd;
          fds[i].events = fds[nfds - 1].events;
          nfds--;
        }
      }
    }
  }
}