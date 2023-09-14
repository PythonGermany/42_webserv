#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>
#include <string>
#include <vector>

#include "Context.hpp"

class Server {
 private:
  std::map<std::string, std::string> _mimeTypes;
  Context _context;
  std::vector<bool> _isDefault;

 public:
  Server();
  Server(const Context &context);
  Server(const Server &rhs);
  Server &operator=(const Server &rhs);
  ~Server();

  // Setters/Adders
  void setMimeTypes(std::map<std::string, std::string> mimeTypes);
  void setContext(const Context &context);

  // Getters
  Context &getContext();

  void print();
};

#endif