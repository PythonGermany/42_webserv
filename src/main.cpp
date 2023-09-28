#include <csignal>

#include "Config.hpp"
#include "Http.hpp"
#include "Init.hpp"
#include "ListenSocket.hpp"
#include "Poll.hpp"
#include "webserv.hpp"

// Parses a flag and its argument
int parseArgument(char flag, char* value) {
  std::string in(value);
  switch (flag) {
    case 's':
      Log::setLogToStdout(in == "on");
      return in != "on" && in != "off";
    case 'l':
      Log::setLevel((t_log_level)(in[0] - '0'));
      return in != "0" && in != "1" && in != "2" && in != "3";
    case 'a':
      std::cout << in << std::endl;
      Log::setLogFile(in);
      return 0;
    case 'e':
      Log::setErrorLogFile(in);
      return 0;
    default:
      return 1;
  }
}

// Loads provided arguments
// @return The selected configuration file path
std::string loadArguments(int argc, char** argv) {
  std::string path = CONFIG_PATH;
  if (argc > 1) {
    if (argv[1][0] != '-') path = std::string(argv[1]);
    for (int i = 1 + (argv[1][0] != '-'); i < argc; i += 2) {
      if (argv[i][0] != '-')
        throw std::runtime_error("Expected '-' at beginning for argument '" +
                                 std::string(argv[i]) + "'");
      if (argv[i][2] != '\0')
        throw std::runtime_error("Expected a flag in the format '-c'");
      if (i + 1 >= argc) throw std::runtime_error("No argument for flag");
      if (parseArgument(argv[i][1], argv[i + 1]))
        throw std::runtime_error("Invalid input for for argument '" +
                                 std::string(argv[i]) + "'");
    }
  }
  return path;
}

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
  struct sigaction pollSignalHandler = {};

  pollSignalHandler.sa_handler = Poll::signalHandler;
  sigaction(SIGINT, &pollSignalHandler, NULL);
  try {
    Init::init(loadConfig(loadArguments(argc, argv)));
    while (true) {
      if (!Poll::poll()) break;
    }
  } catch (const std::exception& e) {
    Log::writeError(e.what(), BRIGHT_YELLOW);
  }
  return 0;
}
