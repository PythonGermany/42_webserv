#include "argument.hpp"

arg_state_t setPrintInfo(const std::list<std::string> &values) {
  static bool set = false;
  if (values.size() > 0) return ARG_INVALID;
  if (set) return FLAG_DUPLICATE;
  printInfo(SET);
  set = true;
  return SUCCESS;
}

arg_state_t setPrintHelp(const std::list<std::string> &values) {
  static bool set = false;
  if (values.size() > 0) return ARG_INVALID;
  if (set) return FLAG_DUPLICATE;
  printHelp(SET);
  set = true;
  return SUCCESS;
}

arg_state_t setPrintConfigStructure(const std::list<std::string> &values) {
  static bool set = false;
  if (values.size() > 0) return ARG_INVALID;
  if (set) return FLAG_DUPLICATE;
  printConfigStructure(SET);
  set = true;
  return SUCCESS;
}

arg_state_t setPrintConfigValidation(const std::list<std::string> &values) {
  static bool set = false;
  if (values.size() > 0) return ARG_INVALID;
  if (set) return FLAG_DUPLICATE;
  printConfigValidation(SET);
  set = true;
  return SUCCESS;
}

arg_state_t setPrintVersion(const std::list<std::string> &values) {
  static bool set = false;
  if (values.size() > 0) return ARG_INVALID;
  if (set) return FLAG_DUPLICATE;
  printVersion(SET);
  set = true;
  return SUCCESS;
}

arg_state_t setLogToTerminal(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  std::string value = values.front();
  Log::setLogToTerminal(value == "on");
  set = true;
  if (value == "on" || value == "off") return SUCCESS;
  return ARG_INVALID;
}

arg_state_t setLogLevel(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  std::string value = values.front();
  Log::setLevel((log_level_t)(value[0] - '0'));
  set = true;
  if (value.size() == 1 && value >= "0" && value <= "4") return SUCCESS;
  return ARG_INVALID;
}

arg_state_t setAccessLog(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  accessLog_g.setFile(values.front());
  accessLog_g.setInitialized(true);
  set = true;
  return SUCCESS;
}

arg_state_t setErrorLog(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  errorLog_g.setFile(values.front());
  errorLog_g.setInitialized(true);
  set = true;
  return SUCCESS;
}

// Finds the argument defining struct for a flag
static arg_t findArgument(char flag) {
  for (size_t i = 0; i < sizeof(args_g) / sizeof(arg_t); i++)
    if (args_g[i].flag == flag) return args_g[i];
  printHelp(SET);
  throw std::runtime_error("Unknown flag '" + toString(flag) + "'");
}

// Loads provided arguments
// @return Config path input or CONFIG_PATH if none was provided
std::string loadArguments(int argc, char **argv) {
  std::string path = CONFIG_PATH;
  if (argc > 1) {
    if (argv[1][0] != '-') path = std::string(argv[1]);
    arg_t arg = {'\0', 0, NULL};
    for (int i = 1 + (argv[1][0] != '-'); i < argc; i += 1 + arg.argCount) {
      if (argv[i][0] != '-')
        throw std::runtime_error("Expected '-' at beginning of argument '" +
                                 std::string(argv[i]) + "'");
      if (argv[i][2] != '\0')
        throw std::runtime_error("Expected a flag in the format '-c'");

      // Find flag structure
      arg = findArgument(argv[i][1]);
      if (i + arg.argCount >= argc)
        throw std::runtime_error("Not enough arguments for flag '" +
                                 std::string(argv[i]) + "'");

      // Generate argument list for flag
      std::list<std::string> args;
      for (int j = i + 1; j < argc && j <= i + arg.argCount; j++)
        args.push_back(argv[j]);

      // Execute flag function
      arg_state_t ret = arg.func(args);
      if (ret == FLAG_DUPLICATE)
        throw std::runtime_error("Duplicate flag '" + std::string(argv[i]) +
                                 "'");
      else if (ret == ARG_INVALID) {
        printHelp(SET);
        throw std::runtime_error("Invalid argument for for flag '" +
                                 std::string(argv[i]) + "'");
      }
    }
  }
  return path;
}
