#include <iostream>
#include <string>

#include "Config.hpp"

int main() {
  Config config("server.conf");
  std::vector<Server> servers;

  try {
    servers = config.parseConfig();
    config.validateConfig(servers);
    for (std::vector<Server>::iterator it = servers.begin();
         it != servers.end(); it++)
      it->print();
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    return 1;
  }
  return 0;
}