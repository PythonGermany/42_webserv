#ifndef SERVER_HPP
#define SERVER_HPP

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "Context.hpp"
#include "utils.hpp"

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
  Context getContext();
  bool getIsDefault();

  void print();
};

#endif