#include <dirent.h>
#include <sys/poll.h>

#include <iostream>
#include <sstream>

#include "File.hpp"
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
  server.setSocket(&sockets[0]);
  server.setHost("localhost");
  server.setPort(atoi(argv[1]));
  server.addName("localhost");
  server.addErrorPage("404", "404.html");
  server.setClientMaxBodySize(1024);

  // Initialize server location directive (TEST)
  location location;
  location._path = "/";
  location._redirect = "";
  location._root = ROOT_PATH;
  location._index = INDEX_PATH;
  location._autoindex = true;
  location._cgi["php"] = "/usr/bin/php-cgi";
  location._cgi["py"] = "/usr/bin/python3";
  server.addLocation(location);

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
            struct location location = server.resolveLocation(request.uri());
            std::string path = location._root + request.uri();
            if (request.uri() == location._path) path += location._index;
            File file(path);

            // Create response
            if (file.exists() == false) {
              response = Response("404", "Not Found");
            } else if (file.readable() == false) {
              response = Response("403", "Forbidden");
            } else if (file.file() == true) {
              response = Response("200", "OK");
              response.setBody(path);
            } else if (file.dir() == true) {
              if (path[path.size() - 1] != '/') {
                response = Response("301", "Moved Permanently");
                response.set_field("Location", request.uri() + "/");
              } else if (location._autoindex == true) {
                response = Response("200", "OK");
                response.set_field("Content-Type", "text/html");
                // response.setBody(
                //     generate_directive_listing(location._root,
                //     request.uri()));
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