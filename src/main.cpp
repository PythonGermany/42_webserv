#include <csignal>

#include "Config.hpp"
#include "Http.hpp"
#include "Init.hpp"
#include "ListenSocket.hpp"
#include "Poll.hpp"
#include "argument.hpp"
#include "global.hpp"

// Loads the config file into a Context object
Context loadConfig(std::string path) {
  Context context("_", NULL);
  Config config(path);
  config.removeComments();
  context = config.parseContext(context, config.getConfig());
  if (LOG_LEVEL >= DEBUG) context.print();
  return context;
}

int main(int argc, char** argv) {
  try {
    std::string path = loadArguments(argc, argv);
    printInfo(PRINT | UNSET);
    if (printHelp(PRINT | UNSET)) return 0;
    Init::init(loadConfig(path));
    while (true) {
      if (!Poll::poll()) break;
    }
  } catch (const std::exception& e) {
    errorLog_g.write(e.what(), ERROR, BRIGHT_RED);
    printHelp(PRINT);
  }
  return 0;
}
