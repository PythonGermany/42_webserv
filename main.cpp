#include <sys/poll.h>

#include <iostream>
#include <set>

#include "Config.hpp"
#include "File.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Socket.hpp"
#include "colors.hpp"

#define CONFIG_FILE "server.conf"
#define MAX_CLIENTS 1

int main(int argc, char** argv) {
  Config config;
  std::vector<Server> servers;
  std::vector<Socket> sockets;
  std::set<std::string> ports;

  try {
    if (argc > 1)
      config.setFile(argv[1]);
    else
      config.setFile(CONFIG_FILE);
    servers = config.parseConfig();
    std::cout << "Config parsing complete" << std::endl;
    config.validateConfig(servers);
    std::cout << "Config validation complete" << std::endl;
    config.setDefaultServers(servers);
    std::cout << "Default servers set" << std::endl;

    for (size_t i = 0; i < servers.size(); i++) {
      if (ports.find(servers[i].getPort()) == ports.end()) {
        ports.insert(servers[i].getPort());
        // Initialize server socket
        Socket socket =
            Socket(servers[i].getHost(), servers[i].getPort(), MAX_CLIENTS);
        int port_reuse = 1;
        socket.Setsockopt(SOL_SOCKET, SO_REUSEADDR, &port_reuse,
                          sizeof(port_reuse));
        sockets.push_back(socket);
        std::cout << "Socket created on " << servers[i].getHost() << ":"
                  << servers[i].getPort() << std::endl;
      }
      std::cout << "Server listening on " << servers[i].getHost() << ":"
                << servers[i].getPort();
      if (servers[i].getIsDefault() == true) std::cout << " (default)";
      std::cout << std::endl;
    }
    std::cout << "Servers listening on " << ports.size() << " port"
              << (ports.size() > 1 ? "s" : "") << std::endl;
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return 1;
  }

  int nfds = ports.size();
  struct sockaddr_in address;
  socklen_t address_len = sizeof(address);
  struct pollfd* fds = new struct pollfd[MAX_CLIENTS * nfds + nfds];
  std::string* request_ports = new std::string[MAX_CLIENTS * nfds + nfds];
  try {
    for (int i = 0; i < nfds; i++) {
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
          request_ports[nfds] = sockets[i].getPort();
          nfds++;
        } else {
          Request request;
          Response response;
          try {
            // Read request
            request = Request(fds[i].fd);
            try {
              std::string host = request.field("Host");
              std::cout << getTimeStamp() << " webserv: Request '" << GREEN
                        << host << RESET << "' from '" << YELLOW
                        << inet_ntoa(address.sin_addr.s_addr) << RESET
                        << "': " << request.method() << " " << request.uri()
                        << " " << request.version() << RED << " -> " << RESET;
            } catch (const std::exception& e) {
              std::string time = getTimeStamp();
              std::cerr << RED << " " << time << e.what() << std::endl << RESET;
            }

            // Get server from host header
            std::string host;
            try {
              host = request.field("Host");
            } catch (const std::exception& e) {
              std::string time = getTimeStamp();
              std::cerr << RED << " " << time << e.what() << std::endl << RESET;
            }
            if (host.find(':') != std::string::npos)
              host = host.substr(0, host.find(':'));
            Server* server = NULL;
            for (size_t j = 0; j < servers.size(); j++) {
              if (servers[j].getPort() == request_ports[i]) {
                std::vector<std::string> names = servers[j].getNames();
                if (std::find(names.begin(), names.end(), host) !=
                    names.end()) {
                  server = &servers[j];
                  break;
                }
              }
            }
            if (server == NULL) server = &servers[0];

            // Get path on server from location directive
            struct location location = server->matchLocation(request.uri());
            File file(location.root + request.uri());

            // Check if index file is available
            if (request.uri() == location.path) {
              for (size_t i = 0; i < location._index.size(); i++) {
                file.setPath(location.root + location._index[i]);
                if (file.exists() && file.readable() && file.file()) break;
              }
            }

            // Create response
            if (location.redirect != "") {
              response = Response("301", "Moved Permanently");
              response.set_field("Location", location.redirect);
            } else if (file.exists() == false) {
              response = Response("404", "Not Found");
            } else if (file.readable() == false) {
              response = Response("403", "Forbidden");
            } else if (file.file() == true) {
              response = Response("200", "OK");
              response.setBody(file.Read());
            } else if (file.dir() == true) {
              std::string fpath = file.path();
              if (endsWith(fpath, "/")) {
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

          std::cout << "Response: " << response.getStatus();

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