#ifndef STRUCTURE_HPP
#define STRUCTURE_HPP

#include <iostream>

typedef enum e_log_level { DEBUG, INFO, WARNING } t_log_level;

typedef struct s_token {
  std::string name;
  std::string parent;
  bool isContext;
  size_t minOccurence;
  size_t maxOccurence;
  size_t minArgs;
  size_t maxArgs;
} t_token;

#endif