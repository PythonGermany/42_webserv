#ifndef STRUCTURE_HPP
#define STRUCTURE_HPP

// Holds the declaration of the webserv structure

#include <iostream>

typedef enum e_log_level { ERROR, WARNING, INFO, DEBUG } t_log_level;

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

// Validation functions are supposed to return a non-empty string if value is
// invalid

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