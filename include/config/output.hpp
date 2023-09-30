#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <iostream>

#include "colors.hpp"

// Information functions which can be set through flags. They need to be
// called once with set = true in order to print out any text should they be
// called again with set = false

void printInfo(bool set = false);
int printHelp(bool set = false);

#endif