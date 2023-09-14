#ifndef INIT_HPP
#define INIT_HPP

#include <vector>

#include "Log.hpp"
#include "Server.hpp"

class Init {
 public:
  Init();
  ~Init();

  static std::vector<Server> initServers(
      Context& context, std::map<std::string, std::string>& mimeTypes);
  static std::map<std::string, std::string> initMimeTypes(Context& context);
  static void initLogDefaults(Context& context);
};

#endif