#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Socket.hpp"

// Struct to store the location block
struct location {
  std::string _path;
  std::vector<std::string> _methods;
  std::string _redirect;
  std::string _root;
  std::vector<std::string> _index;
  bool _autoindex;
  std::map<std::string, std::string> _cgi;
};

// Class to store the server block
class Server {
 private:
  std::string _host;
  int _port;
  std::vector<std::string> _names;
  std::map<std::string, std::string> _error_pages;
  int _client_max_body_size;
  std::vector<location> _locations;
  bool _isDefault;

 public:
  Server();
  ~Server();

  // Setters/Adders
  void setHost(std::string host);
  void setPort(int port);
  void addName(std::string name);
  void setNames(std::vector<std::string> names);
  void addErrorPage(std::string code, std::string path);
  void setClientMaxBodySize(int size);
  void addLocation(location location);
  void setIsDefault(bool idDefault);

  // Getters
  std::string getHost();
  int getPort();
  std::vector<std::string> getNames();
  std::map<std::string, std::string> getErrorPages();
  int getClientMaxBodySize();
  std::vector<location> getLocations();
  bool getIsDefault();

  // Resolves the longest matching location for a path
  // @param path The path to match
  // @return The longest matching location
  // @exception std::runtime_error Thrown if no matching location is found
  location matchLocation(std::string path);

  // Prints the current server block configuration
  void print();
};

#endif