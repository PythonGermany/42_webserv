#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/stat.h>

#include <string>

std::string trim(std::string str);
std::string cut(std::string &str, int start, int end);
long int size(std::string path);
long int modified(std::string path);

#endif