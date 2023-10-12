#ifndef ARGUMENT_HPP
#define ARGUMENT_HPP

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
arg_state_e setInfo(const std::list<std::string> &values);
arg_state_e setHelp(const std::list<std::string> &values);
arg_state_t setLogToStdout(const std::list<std::string> &values);
arg_state_t setLogLevel(const std::list<std::string> &values);
arg_state_t setAccessLog(const std::list<std::string> &values);
arg_state_t setErrorLog(const std::list<std::string> &values);

// WEBSERV_CONFIG ----------- ARG VALUES ---------------------------
/**
 * Allowed argument input structure in the format: {flag, argCount,
 * validation-/initializationFunction}
 */
const arg_t args_g[6] = {{'i', 0, setInfo},        {'h', 0, setHelp},
                         {'s', 1, setLogToStdout}, {'l', 1, setLogLevel},
                         {'a', 1, setAccessLog},   {'e', 1, setErrorLog}};

std::string loadArguments(int argc, char **argv);

#endif