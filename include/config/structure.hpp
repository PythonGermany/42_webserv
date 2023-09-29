#ifndef STRUCTURE_HPP
#define STRUCTURE_HPP

#include <string>

// ------------------------- LOG STRUCTURE -------------------------

typedef enum e_log_level { ERROR, WARNING, INFO, DEBUG } t_log_level;

// ------------------------- ARG STRUCTURE -------------------------

typedef enum arg_state_e {
  SUCCESS,
  FLAG_UNKNOWN,
  FLAG_DUPLICATE,
  ARG_INVALID,
} arg_state_t;

typedef struct arg_s {
  char flag;
  arg_state_t (*func)(std::string);
} arg_t;

// Validation functions return either SUCCESS or another value fron the enum to
// indicate an error

arg_state_t setLogToStdout(std::string value);
arg_state_t setLogLevel(std::string value);
arg_state_t setAccessLog(std::string value);
arg_state_t setErrorLog(std::string value);

// ----------------------- CONFIG STRUCTURE ------------------------

typedef struct s_token {
  std::string name;
  std::string parent;
  bool isContext;
  size_t minOccurence;
  size_t maxOccurence;
  size_t minArgs;
  size_t maxArgs;
  std::string (*func)(std::string const &value, size_t index);
} t_token;

// Validation functions are return a non-empty string if value is invalid

std::string isMimeType(std::string const &value, size_t index);
std::string isErrorPage(std::string const &value, size_t index);
std::string isNumeric(std::string const &value, size_t index);
std::string isMethod(std::string const &value, size_t index);
std::string isLogLevel(std::string const &value, size_t index);
std::string isAbsolutePath(std::string const &value, size_t index);
std::string isExtension(std::string const &value, size_t index);
std::string isBoolean(std::string const &value, size_t index);
std::string isListen(std::string const &value, size_t index);

#endif