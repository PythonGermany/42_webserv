#ifndef UTILS_HPP
#define UTILS_HPP

#include <sys/stat.h>

#include <string>
#include <vector>

// Trims thhe start and end of the string from whitespace
// @param str The string to trim
// @return The trimmed string
std::string trim(std::string str);

// Removes the substring from start to end from str and returns it
// @param str The string to cut from
// @param start The start index of the substring to cut
// @param end The end index of the substring to cut
// @return The cut substring
// @note str is modified
std::string cut(std::string &str, int start, int end);

// Splits the string by the delimiter
// @param str The string to split
// @param delim The delimiter to split by
// @return The substrings
std::vector<std::string> split(std::string str, std::string delim);

// Checks if the string is numeric
// @param str The string to check
// @return True if the string is numeric, false otherwise
bool isNumeric(std::string str);

long int size(std::string path);
long int modified(std::string path);

#endif