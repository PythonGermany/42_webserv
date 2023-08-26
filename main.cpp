#include <dirent.h>
#include <sys/poll.h>

#include <iostream>
#include <sstream>

#include "Request.hpp"
#include "Response.hpp"
#include "Server.hpp"
#include "Socket.hpp"

#define MAX_CLIENTS 10
#define ROOT_PATH "/home/pythongermany/_Projects/42_webserv/website"
#define INDEX_PATH "index.html"

template <typename T>
std::string to_string(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

std::string generate_directive_listing(std::string root, std::string path) {
  std::string host_path = root + path;
  std::string listing = "<html><head><title>Index of " + path +
                        "</title></head><body><h1>Index of " + path +
                        "</h1><hr><pre>";
  DIR* dir = opendir(host_path.c_str());
  if (dir == NULL)
    throw std::runtime_error("generate_directive_listing: opendir error");
  struct dirent* entry = readdir(dir);
  while (entry != NULL) {
    std::string fsize = to_string(size(host_path + entry->d_name));
    std::string fmodified = to_string(modified(host_path + entry->d_name));
    listing += "<a href=\"" + path + std::string(entry->d_name) + "\">" +
               std::string(entry->d_name) + "</a>" + "    " + fmodified +
               "    " + fsize + "<br>";
    entry = readdir(dir);
  }
  closedir(dir);
  listing += "</pre><hr></body></html>";
  return listing;
}

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
  try {
    for (int i = 0; i < argc - 1; i++) {
      // Initialize server socket
      sockets[i] = Socket(AF_INET, SOCK_STREAM, 0);
      int port_reuse = 1;
      sockets[i].Setsockopt(SOL_SOCKET, SO_REUSEADDR, &port_reuse,
                            sizeof(port_reuse));
      sockets[i].Bind(atoi(argv[i + 1]));
      sockets[i].Listen(MAX_CLIENTS);
      std::cout << "Listening on port " << argv[i + 1] << std::endl;

      // Initialize pollfd struct
      fds[i].fd = sockets[i].fd();
      fds[i].events = POLLIN;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
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
  locations[0]._autoindex = true;
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

            // Get path on server from location directive
            location location = server.resolve_location(request.uri());
            std::string path = location._root + request.uri();
            if (request.uri() == location._path) path += location._index;

            // Create response
            if (exists(path) == false) {
              response = Response("404", "Not Found");
            } else if (is_readable(path) == false) {
              response = Response("403", "Forbidden");
            } else if (is_file(path) == true) {
              response = Response("200", "OK");
              response.load_body(path);
            } else if (is_dir(path) == true) {
              if (path[path.size() - 1] != '/') {
                response = Response("301", "Moved Permanently");
                response.set_field("Location", request.uri() + "/");
              } else if (location._autoindex == true) {
                response = Response("200", "OK");
                response.set_field("Content-Type", "text/html");
                response.set_body(
                    generate_directive_listing(location._root, request.uri()));
              } else {
                response = Response("403", "Forbidden");
              }
            } else {
              response = Response("500", "Internal Server Error");
            }
          } catch (const std::exception& e) {
            std::cerr << "main: " << e.what() << std::endl;
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