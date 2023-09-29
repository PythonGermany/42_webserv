#ifndef ARGUMENT_HPP
#define ARGUMENT_HPP

#include "structure.hpp"
#include "webserv.hpp"

int parseArgument(char flag, std::string value);
std::string loadArguments(int argc, char** argv);

#endif