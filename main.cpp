#include <sys/poll.h>

#include <iostream>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Socket.hpp"

#define MAX_CLIENTS 10
#define ROOT_PATH "/home/pythongermany/_Projects/42_webserv/website"
#define INDEX_PATH "index.html"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "usage: " << argv[0] << " <port> [<port> ...]" << std::endl;
    return 1;
  }

  int nfds = argc - 1;
  Socket sockets[nfds];
  Server server;
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
      std::cerr << e.what() << std::endl;
      return 1;
    }
    std::cout << "Listening on port " << argv[i + 1] << std::endl;

    // Initialize pollfd struct
    fds[i].fd = sockets[i].fd();
    fds[i].events = POLLIN;
  }

  // Initialize server values (TEST)
  server.set_socket(&sockets[0]);  // Unused at the moment
  server.set_host("localhost");    // Unused at the moment
  server.set_port(atoi(argv[1]));  // Unused at the moment
  server.set_names(
      std::vector<std::string>(1, "localhost"));  // Unused at the moment
  server.set_error_pages(
      std::map<std::string, std::string>());  // Unused at the moment
  server.set_client_max_body_size(1024);      // Unused at the moment

  // Initialize server location directive (TEST)
  std::vector<location> locations;
  locations.push_back(location());
  locations[0]._path = "/";
  locations[0]._redirect = "";
  locations[0]._root = ROOT_PATH;
  locations[0]._index = INDEX_PATH;
  locations[0]._autoindex = false;
  locations[0]._cgi_path = "";
  locations[0]._cgi_extension = "php";
  server.set_locations(locations);

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

            // Get path from location directive
            location location = server.resolve_location(request.uri());
            std::string file_path = location._root + request.uri();
            if (is_dir(file_path)) file_path += location._index;

            // Create response
            if (is_file(file_path) && is_readable(file_path)) {
              response = Response("200", "OK");
              response.load_body(file_path);
            } else
              response = Response("404", "Not Found");
          } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
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