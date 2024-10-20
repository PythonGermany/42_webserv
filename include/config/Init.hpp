#ifndef INIT_HPP
#define INIT_HPP

#include "Context.hpp"

class Init {
 private:
  Init();
  ~Init();

 public:
  static void init(Context context);
  static void initLogDefaults(Context& context);
  static void initMimeTypes(Context& context);
  static void initVirtualHosts(Context& context);
  static void initAConnection(Context& context);
  static void initPoll();
};

#endif
