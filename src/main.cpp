#include <csignal>

#include "Config.hpp"
#include "Http.hpp"
#include "Init.hpp"
#include "ListenSocket.hpp"
#include "Poll.hpp"
#include "webserv.hpp"

Context loadConfig(int argc, char** argv) {
  if (argc > 2) Log::setLogToStdout(std::string(argv[2]) == "on");
  if (argc > 3) Log::setLevel((t_log_level)(*argv[3] - '0'));
  Log::write("-------- Loading config file --------", INFO, BRIGHT_GREEN);
  Context context("_", NULL);
  Config config(argc > 1 ? argv[1] : CONFIG_PATH);
  config.removeComments();
  context = config.parseContext(context, config.getConfig());
  if (LOG_LEVEL >= DEBUG) context.print();
  return context;
}

int main(int argc, char** argv) {
  struct sigaction pollSignalHandler = {};

  pollSignalHandler.sa_handler = Poll::signalHandler;
  sigaction(SIGINT, &pollSignalHandler, NULL);
  try {
    Context context = loadConfig(argc, argv);
    Init::init(context);
    while (true) {
      if (!Poll::poll()) break;
    }
  } catch (const std::exception& e) {
    Log::writeError(e.what(), BRIGHT_YELLOW);
  }
  return 0;
}
