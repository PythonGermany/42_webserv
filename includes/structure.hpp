#ifndef STRUCTURE_HPP
#define STRUCTURE_HPP

#include <iostream>

typedef enum e_log_level { WARNING, INFO, DEBUG } t_log_level;

typedef struct s_token {
  std::string name;
  std::string parent;
  bool isContext;
  size_t minOccurence;
  size_t maxOccurence;
  size_t minArgs;
  size_t maxArgs;
  std::string (*func)(std::string const &value);
} t_token;

std::string isNumeric(std::string const &value);

std::string isMethod(std::string const &value);

std::string isLogLevel(std::string const &value);

std::string isAbsolutePath(std::string const &value);

std::string isBoolean(std::string const &value);

#endif