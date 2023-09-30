#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <iostream>

#include "colors.hpp"

typedef enum output_e { PRINT, SET, UNSET, PRINT_IF_SET } output_t;

// Information output functions can be triggered through flags. They need to be
// called once with set = true in order to print out any text should they be
// called again with set = false

void printInfo(output_t command);
int printHelp(output_t command);

#endif