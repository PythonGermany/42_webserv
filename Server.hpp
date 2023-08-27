#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "Socket.hpp"

struct location {
  std::string _path;
  std::vector<std::string> _methods;
  std::string _redirect;
  std::string _root;
  std::string _index;
  bool _autoindex;
  std::map<std::string, std::string> _cgi;
};

class Server {
 private:
  Socket *_socket;
  std::string _host;
  int _port;
  std::vector<std::string> _names;
  std::map<std::string, std::string> _error_pages;
  int _client_max_body_size;
  std::vector<location> _locations;

 public:
  Server();
  ~Server();

  // TODO: Add setters

  void addLocation(location location);

  location resolveLocation(std::string path);
};

#endif