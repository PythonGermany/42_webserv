#ifndef ARGUMENT_HPP
#define ARGUMENT_HPP

#include "structure.hpp"
#include "webserv.hpp"

arg_state_t parseArgument(char flag, std::string value);
std::string loadArguments(int argc, char** argv);

#endif