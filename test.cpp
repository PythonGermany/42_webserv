#include <iostream>
#include <string>

#include "Config.hpp"

int main(int argc, char** argv) {
  Config config("server_example.conf");
  std::vector<Server> servers = config.parse();
}