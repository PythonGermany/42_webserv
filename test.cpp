#include <iostream>
#include <string>

#include "Config.hpp"

int main() {
  Config config("server_example.conf");
  std::vector<Server> servers;

  try {
    servers = config.parse();
    config.validate(servers);
    for (std::vector<Server>::iterator it = servers.begin();
         it != servers.end(); it++)
      it->print();
  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}