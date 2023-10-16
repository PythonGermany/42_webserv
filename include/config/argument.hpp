#ifndef ARGUMENT_HPP
#define ARGUMENT_HPP

#include "Config.hpp"
#include "global.hpp"
#include "output.hpp"

// ------------------------- ARG STRUCTURE -------------------------
typedef enum arg_state_e {
  SUCCESS,
  FLAG_UNKNOWN,
  FLAG_DUPLICATE,
  ARG_INVALID,
} arg_state_t;

typedef struct arg_s {
  char flag;
  int argCount;
  arg_state_t (*func)(const std::list<std::string> &);
} arg_t;

// Validation functions return either SUCCESS or another value fron the enum to
// indicate an error
arg_state_e setPrintInfo(const std::list<std::string> &values);
arg_state_e setPrintHelp(const std::list<std::string> &values);
arg_state_t setPrintConfigStructure(const std::list<std::string> &values);
arg_state_t setPrintConfigValidation(const std::list<std::string> &values);
arg_state_t setPrintVersion(const std::list<std::string> &values);
arg_state_t setLogToTerminal(const std::list<std::string> &values);
arg_state_t setLogLevel(const std::list<std::string> &values);
arg_state_t setAccessLog(const std::list<std::string> &values);
arg_state_t setErrorLog(const std::list<std::string> &values);

// WEBSERV_CONFIG ----------- ARG VALUES ---------------------------
/**
 * Allowed argument input structure in the format: {flag, argCount,
 * validation-/initializationFunction}
 */
const arg_t args_g[9] = {{'i', 0, setPrintInfo},
                         {'h', 0, setPrintHelp},
                         {'v', 0, setPrintVersion},
                         {'c', 0, setPrintConfigStructure},
                         {'t', 0, setPrintConfigValidation},
                         {'s', 1, setLogToTerminal},
                         {'l', 1, setLogLevel},
                         {'a', 1, setAccessLog},
                         {'e', 1, setErrorLog}};

std::string loadArguments(int argc, char **argv);

#endif