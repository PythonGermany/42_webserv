#include "argument.hpp"

// Finds the argument defining struct for a flag
arg_t findArgument(char flag) {
  for (size_t i = 0; i < sizeof(args_g) / sizeof(arg_t); i++)
    if (args_g[i].flag == flag) return args_g[i];
  throw std::runtime_error("Unknown flag '" + toString(flag) +
                           "'. See 'webserv -h'");
}

// Loads provided arguments
// @return The selected configuration file path
std::string loadArguments(int argc, char** argv) {
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
      else if (ret == ARG_INVALID)
        throw std::runtime_error("Invalid argument for for flag '" +
                                 std::string(argv[i]) + "'");
    }
  }
  return path;
}