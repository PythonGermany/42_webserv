#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Socket.hpp"

struct location {
  std::string _path;
  std::vector<std::string> _methods;
  std::string _redirect;
  std::string _root;
  std::vector<std::string> _index;
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

  void setSocket(Socket *socket);
  void setHost(std::string host);
  void setPort(int port);
  void addName(std::string name);
  void setNames(std::vector<std::string> names);
  void addErrorPage(std::string code, std::string path);
  void setClientMaxBodySize(int size);
  void addLocation(location location);

  Socket *getSocket();
  std::string getHost();
  int getPort();
  std::vector<std::string> getNames();
  std::map<std::string, std::string> getErrorPages();
  int getClientMaxBodySize();
  std::vector<location> getLocations();

  location resolveLocation(std::string path);

  void print();
};

#endif