#include "Config.hpp"
#include "Init.hpp"
#include "ListenSocket.hpp"
#include "Poll.hpp"
#include "webserv.hpp"
#include <csignal>

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

/**
 * TODO: Log::write() and toString() can throw std::bad_alloc because they use std::string
*/
int main(int argc, char** argv) {
  struct sigaction pollSignalHandler = {};

  pollSignalHandler.sa_handler = Poll::signalHandler;
  sigaction(SIGINT, &pollSignalHandler, NULL);
  try {
    Context context = loadConfig(argc > 1 ? argv[1] : CONFIG_PATH);
    Init::init(context);
    while (true)
    {
      if (!Poll::poll())
        break;
    }
  } catch (const std::exception& e) {
    Log::writeError(e.what(), BRIGHT_YELLOW);
  }
  Log::close();
  Log::write("Number of open files: " + toString(File::getFilesOpen()), INFO);
  return 0;
}
