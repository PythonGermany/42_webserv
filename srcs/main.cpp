#include "Config.hpp"
#include "webserv.hpp"

int main(int argc, char** argv) {
  Config config;
  std::vector<Server> servers;

  writeToLog("-------- Loading config file --------", INFO, BRIGHT_GREEN);
  try {
    if (argc > 1)
      config = Config(argv[1]);
    else
      config = Config(CONFIG_PATH);
    servers = config.parseConfig();
    config.validateConfig(servers);
  } catch (std::exception& e) {
    return 1;
  }
  writeToLog("-- Config file successfully loaded --", INFO, BRIGHT_GREEN);
  if (LOG_LEVEL <= DEBUG) {
    for (std::vector<Server>::iterator it = servers.begin();
         it != servers.end(); it++) {
      it->print();
      std::cout << std::endl;
    }
  }
  return 0;
}