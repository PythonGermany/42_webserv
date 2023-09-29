#include "argument.hpp"

// Parses a flag and its argument
int parseArgument(char flag, std::string value) {
  for (size_t i = 0; i < sizeof(args_g) / sizeof(arg_t); i++)
    if (args_g[i].flag == flag) return args_g[i].func(value);
  return FLAG_UNKNOWN;
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
      int ret = parseArgument(argv[i][1], argv[i + 1]);
      if (ret == SUCCESS) continue;
      if (ret == FLAG_UNKNOWN)
        throw std::runtime_error("Unknown flag '" + std::string(argv[i]) + "'");
      else if (ret == FLAG_DUPLICATE)
        throw std::runtime_error("Duplicate flag '" + std::string(argv[i]) +
                                 "'");
      else if (ret == ARG_INVALID)
        throw std::runtime_error("Invalid argument for for flag '" +
                                 std::string(argv[i]) + "'");
    }
  }
  return path;
}