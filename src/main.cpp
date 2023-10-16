#include <csignal>

#include "Config.hpp"
#include "Http.hpp"
#include "Init.hpp"
#include "ListenSocket.hpp"
#include "Poll.hpp"
#include "argument.hpp"
#include "global.hpp"

void setupGlobals() {
  cwd_g = getcwd();
  if (cwd_g.empty()) throw std::runtime_error("getcwd(): Failed to init pwd");
  try {
    accessLog_g.init(LOG_PATH);
    errorLog_g.init(LOG_ERROR_PATH);
  } catch (...) {
    throw std::runtime_error("Failed to init logfile defaults");
  }
}

// Loads the config file into a Context object
Context loadConfig(std::string path) {
  Context context("_", NULL);
  Config config(path);
  config.removeComments();
  context = config.parseContext(context, config.getConfig());
  return context;
}

int main(int argc, char** argv) {
  try {
    setupGlobals();

    std::string path = loadArguments(argc, argv);
    printInfo(PRINT | UNSET);

    int ret = 0;
    ret |= printVersion(PRINT | UNSET);
    ret |= printHelp(PRINT | UNSET);
    if (ret) return 0;

    Context context = loadConfig(path);
    ret |= printConfigStructure(PRINT | UNSET, context.getStructure());
    ret |= printConfigValidation(PRINT | UNSET, path);
    if (ret) return 0;
    Init::init(context);
    while (true) {
      if (!Poll::poll()) break;
    }
  } catch (const std::exception& e) {
    errorLog_g.write(e.what(), ERROR);
    printHelp(PRINT);
    return 1;
  }
  return 0;
}
