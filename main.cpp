#include <fcntl.h>
#include <sys/poll.h>

#include <cstdlib>
#include <cstring>
#include <iostream>

#include "Request.hpp"
#include "Response.hpp"
#include "Socket.hpp"

#define MAX_CLIENTS 10
#define ROOT_PATH "/home/pythongermany/_Projects/42_webserv/website"
#define INDEX_PATH "/index.html"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " <port> [<port> ...]" << std::endl;
    return 1;
  }

  int nfds = argc - 1;
  Socket sockets[nfds];
  struct sockaddr_in address;
  socklen_t address_len = sizeof(address);
  struct pollfd fds[MAX_CLIENTS * nfds + nfds];

  memset(fds, 0, sizeof(fds));
  for (int i = 0; i < argc - 1; i++) {
    try {
      // Initialize server socket
      sockets[i] = Socket(AF_INET, SOCK_STREAM, 0);
      int port_reuse = 1;
      sockets[i].Setsockopt(SOL_SOCKET, SO_REUSEADDR, &port_reuse,
                            sizeof(port_reuse));
      sockets[i].Bind(atoi(argv[i + 1]));
      sockets[i].Listen(MAX_CLIENTS);
    } catch (const std::exception& e) {
      std::cerr << "init: " << sockets[i].get_error() << std::endl;
      return 1;
    }
    std::cout << "Listening on port " << argv[i + 1] << std::endl;

    // Initialize pollfd struct
    fds[i].fd = sockets[i].get_fd();
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
        if (fds[i].fd == sockets[i].get_fd()) {
          int client_df =
              accept(sockets[i].get_fd(), (sockaddr*)&address, &address_len);
          if (client_df < 0) {
            std::cerr << "accept error: " << strerror(errno) << std::endl;
            return 1;
          }

          fds[nfds].fd = client_df;
          fds[nfds].events = POLLIN;
          nfds++;
        } else {
          Request request(fds[i].fd);
          std::cout << "request: " << request.get_method() << " "
                    << request.get_uri() << " " << request.get_version()
                    << std::endl;
          Response response("200", "OK");

          std::string file_path = ROOT_PATH;
          if (request.get_uri() == "/")
            file_path += INDEX_PATH;
          else
            file_path += request.get_uri();
          int file = open(file_path.c_str(), O_RDONLY);
          if (file < 0 || request.get_method() != "GET") {
            response = Response("404", "Not Found");
            response.set_body("404 Not Found");
          } else {
            response.set_body(file);
            close(file);
          }
          int write_result =
              write(fds[i].fd, response.get().c_str(), response.get().size());
          if (write_result < 0) {
            std::cerr << "write error: " << strerror(errno) << std::endl;
            return 1;
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