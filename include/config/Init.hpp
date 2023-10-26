#ifndef INIT_HPP
#define INIT_HPP

#include "ListenSocket.hpp"
#include "Log.hpp"
#include "Poll.hpp"
#include "VirtualHost.hpp"
#include "global.hpp"

class Init {
 private:
  Init();
  ~Init();

 public:
  static void init(Poll& poll, Context& context);
  static void initLogDefaults(Context& context);
  static void initMimeTypes(Context& context);
  static void initVirtualHosts(Context& context);
  static void initPoll(Poll& poll);
};

#endif
