#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include <vector>

#include "Socket.hpp"

struct location {
  std::vector<std::string> _methods;
  std::string _redirect;
  std::string _root;
  std::string _index;
  bool _autoindex;
  std::string _cgi_path;
  std::string _cgi_extension;
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
  Server(std::string config);
  ~Server();
};

#endif