#ifndef STRUCTURE_HPP
#define STRUCTURE_HPP

#include <list>
#include <string>

// ------------------------- LOG STRUCTURE -------------------------

typedef enum log_level_e { ERROR, WARNING, INFO, DEBUG } log_level_t;

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

// Information functions which can be set through flags. They need to be
// called once with set = true in order to print out any text should they be
// called again with set = false

void printInfo(bool set = false);
int printHelp(bool set = false);

// Validation functions return either SUCCESS or another value fron the enum to
// indicate an error

arg_state_e setInfo(const std::list<std::string> &values);
arg_state_e setHelp(const std::list<std::string> &values);
arg_state_t setLogToStdout(const std::list<std::string> &values);
arg_state_t setLogLevel(const std::list<std::string> &values);
arg_state_t setAccessLog(const std::list<std::string> &values);
arg_state_t setErrorLog(const std::list<std::string> &values);

// ----------------------- CONFIG STRUCTURE ------------------------

typedef struct token_s {
  std::string name;
  std::string parent;
  bool isContext;
  size_t minOccurence;
  size_t maxOccurence;
  size_t minArgs;
  size_t maxArgs;
  std::string (*func)(std::string const &value, size_t index);
} token_t;

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