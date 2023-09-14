#ifndef VIRTUALHOST_HPP
#define VIRTUALHOST_HPP

#include <map>
#include <string>
#include <vector>

#include "Context.hpp"

class VirtualHost {
 private:
  static std::map<std::string, std::string> _mimeTypes;
  Context _context;
  std::vector<bool> _isDefault;

 public:
  VirtualHost();
  VirtualHost(const Context &context);
  VirtualHost(const VirtualHost &rhs);
  VirtualHost &operator=(const VirtualHost &rhs);
  ~VirtualHost();

  // Setters/Adders
  static void setMimeTypes(std::map<std::string, std::string> &mimeTypes);
  void setContext(const Context &context);

  void print();
};

#endif