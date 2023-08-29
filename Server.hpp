#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Cgi.hpp"
#include "Context.hpp"
#include "utils.hpp"

// Struct to store the location block
struct location {
  std::string path;
  std::vector<std::string> methods;
  std::string redirect;
  std::string root;
  std::vector<std::string> index;
  bool autoindex;
  std::string upload;
  Cgi cgi;
};

// Class to store the server block
class Server {
 private:
  Context _context;
  bool _isDefault;

 public:
  Server();  // TODO: Add copy constructor
  Server(Context context);
  Server(const Server &rhs);
  Server &operator=(const Server &rhs);
  ~Server();

  // Setters/Adders
  void setContext(Context context);
  void setIsDefault(bool idDefault);

  // Getters
  Context &getContext();
  bool getIsDefault();

  // Prints the current server block configuration
  void print();
};

#endif