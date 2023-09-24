#include <csignal>

#include "Config.hpp"
#include "Http.hpp"
#include "Init.hpp"
#include "ListenSocket.hpp"
#include "Poll.hpp"
#include "webserv.hpp"

Context loadConfig(std::string path) {
  Log::write("-------- Loading config file --------", INFO, BRIGHT_GREEN);
  Context context("_", NULL);
  Config config(path);
  config.removeComments();
  context = config.parseContext(context, config.getConfig());
  if (LOG_LEVEL >= DEBUG) context.print();
  Log::write("-- Config file successfully loaded --", INFO, BRIGHT_GREEN);
  return context;
}

int main(int argc, char** argv) {
  struct sigaction pollSignalHandler = {};

  pollSignalHandler.sa_handler = Poll::signalHandler;
  sigaction(SIGINT, &pollSignalHandler, NULL);
  try {
    Context context = loadConfig(argc > 1 ? argv[1] : CONFIG_PATH);
    Init::init(context);
    while (true) {
      if (!Poll::poll()) break;
      if (Http::updateCache())
        Log::write("Cache has changed: " + Http::getCache().info(), DEBUG);
    }
  } catch (const std::exception& e) {
    Log::writeError(e.what(), BRIGHT_YELLOW);
  }
  Log::close();
  if (File::getFilesOpen()) return 1;
  return 0;
}
