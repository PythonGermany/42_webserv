#ifndef OUTPUT_HPP
#define OUTPUT_HPP

#include <string>

typedef enum output_flag_e { SET = 1, PRINT = 2, UNSET = 4 } output_flag_t;

// Information output functions can be triggered through flags. They need to be
// called once with set = true in order to print out any text should they be
// called again with set = false

void printInfo(int flags);
int printHelp(int flags);
int printConfigStructure(int flags, std::string data = "");
int printConfigValidation(int flags, std::string path = "");
int printVersion(int flags);

#endif
