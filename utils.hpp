#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/stat.h>

#include <string>

std::string trim(std::string str);
long int size(std::string path);
long int modified(std::string path);

#endif