#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/stat.h>

#include <string>

std::string trim(std::string str);
bool exists(std::string path);
bool is_file(std::string path);
bool is_dir(std::string path);
bool is_readable(std::string path);

#endif