#ifndef INIT_HPP
#define INIT_HPP

#include <set>
#include <vector>

#include "../poll/ListenSocket.hpp"
#include "Log.hpp"
#include "../poll/Poll.hpp"
#include "VirtualHost.hpp"

class Init {
 private:
  Init();
  ~Init();

 public:
  static void init(Context& context);
  static void initVirtualHosts(Context& context);
  static void initMimeTypes(Context& context);
  static void initLogDefaults(Context& context);
  static void initPoll();
};

#endif