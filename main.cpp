#include <sys/poll.h>

#include <iostream>

#include "Config.hpp"
#include "File.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Socket.hpp"

#define CONFIG_FILE "server.conf"
#define MAX_CLIENTS 10

int main(int argc, char** argv) {
  Config config;
  std::vector<Server> servers;

  try {
    if (argc > 1)
      config.setFile(argv[1]);
    else
      config.setFile(CONFIG_FILE);
    servers = config.parseConfig();
    std::cout << "Config parsing complete" << std::endl;
    config.validateConfig(servers);
    std::cout << "Config validation complete" << std::endl;
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  int nfds = 1;
  Socket sockets[nfds];
  struct sockaddr_in address;
  socklen_t address_len = sizeof(address);
  struct pollfd fds[MAX_CLIENTS * nfds + nfds];

  memset(fds, 0, sizeof(fds));
  try {
    for (int i = 0; i < nfds; i++) {
      // Initialize server socket
      sockets[i] = Socket(AF_INET, SOCK_STREAM, 0);
      int port_reuse = 1;
      sockets[i].Setsockopt(SOL_SOCKET, SO_REUSEADDR, &port_reuse,
                            sizeof(port_reuse));
      sockets[i].Bind(servers[0].getPort(), INADDR_ANY);
      sockets[i].Listen(MAX_CLIENTS);
      std::cout << "Listening on port " << servers[0].getPort() << std::endl;

      // Initialize pollfd struct
      fds[i].fd = sockets[i].fd();
      fds[i].events = POLLIN;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  while (true) {
    int poll_result = poll(fds, nfds, -1);
    if (poll_result < 0) {
      std::cerr << "poll error: " << strerror(errno) << std::endl;
      return 1;
    }
    for (int i = 0; i < nfds; i++) {
      if (fds[i].revents & POLLIN) {
        if (fds[i].fd == sockets[i].fd()) {
          int client_df =
              accept(sockets[i].fd(), (sockaddr*)&address, &address_len);
          if (client_df < 0) {
            std::cerr << "accept error: " << strerror(errno) << std::endl;
            return 1;
          }
          fds[nfds].fd = client_df;
          fds[nfds].events = POLLIN;
          nfds++;
        } else {
          Request request;
          Response response;
          try {
            // Read request
            request = Request(fds[i].fd);
            std::cout << "request: " << request.method() << " " << request.uri()
                      << " " << request.version() << std::endl;

            // Get path on server from location directive
            struct location location = servers[0].matchLocation(request.uri());
            File file(location._root + request.uri());

            // Check if file is available
            if (request.uri() == location._path) {
              for (size_t i = 0; i < location._index.size(); i++) {
                file.setPath(location._root + location._index[i]);
                if (file.exists() && file.readable() && file.file()) break;
              }
            }

            // Create response
            if (file.exists() == false) {
              response = Response("404", "Not Found");
            } else if (file.readable() == false) {
              response = Response("403", "Forbidden");
            } else if (file.file() == true) {
              response = Response("200", "OK");
              response.setBody(file.Read());
            } else if (file.dir() == true) {
              std::string fpath = file.path();
              if (fpath[fpath.size() - 1] !=
                  '/') {  // TODO: fix my dumb implementation
                response = Response("301", "Moved Permanently");
                response.set_field("Location", request.uri() + "/");
              } else if (location._autoindex == true) {
                response = Response("500", "Internal Server Error");
              } else
                response = Response("403", "Forbidden");
            } else
              response = Response("500", "Internal Server Error");
          } catch (const std::exception& e) {
            std::cerr << "webserv: " << e.what() << std::endl;
            response = Response("400", "Bad Request");
          }
          response.set_field("Server", "webserv");

          // Send response
          try {
            response.send(fds[i].fd);
          } catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
          }

          fds[i].fd = fds[nfds - 1].fd;
          fds[i].events = fds[nfds - 1].events;
          nfds--;
        }
      }
    }
  }
}