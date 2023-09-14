#ifndef INIT_HPP
#define INIT_HPP

#include <vector>

#include "Log.hpp"
#include "VirtualHost.hpp"

class Init {
 public:
  Init();
  ~Init();

  static std::vector<VirtualHost> initVirtualHosts(Context& context);
  static void initMimeTypes(Context& context);
  static void initLogDefaults(Context& context);
};

#endif