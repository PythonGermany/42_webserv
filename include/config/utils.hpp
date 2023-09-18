#ifndef UTILS_HPP
#define UTILS_HPP

#include <dirent.h>
#include <fnmatch.h>
#include <netdb.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstring>
#include <sstream>
#include <vector>

#include <stdio.h>

#include "colors.hpp"

// Trims start and end of a string
// @param str The string to trim
// @param chars The characters to trim
// @return The trimmed string
// @exception No custom exceptions
std::string trim(const std::string& str, std::string chars = " \f\n\r\t\v");

// Trim the start of a string
// @param str The string to trim
// @param chars The characters to trim
// @return The trimmed string
// @exception No custom exceptions
std::string& trimStart(std::string& str, std::string chars = " \f\n\r\t\v");

// Cuts a string from start to end
// @param str The string to cut from
// @param start The start index
// @param end The end index
// @return The cut string
// @exception No custom exceptions
std::string cut(std::string& str, int start, int end);

// Splits a string into a vector of strings
// @param str The string to split
// @param delim The delimiter characters
// @return The vector of strings
std::vector<std::string> split(const std::string& str, std::string delim);

// Checks if a string starts with a prefix
// @param str The string to check
// @param prefix The prefix to check for
// @return True if the string starts with the prefix, false otherwise
// @exception No custom exceptions
bool startsWith(std::string str, std::string prefix);

// Checks if a string ends with a suffix
// @param str The string to check
// @param suffix The suffix to check for
// @return True if the string ends with the suffix, false otherwise
// @exception No custom exceptions
bool endsWith(std::string str, std::string suffix);

// Converts a value to a string
// @param val The value to convert
// @return The string
// @exception No custom exceptions
template <typename T>
std::string toString(T val) {
  std::stringstream ss;
  ss << val;
  return ss.str();
}

std::string toHexString(unsigned char c);

// Converts an integer value to a string in the IPv4 format
// @param addr The integer value to convert
// @return The string
// @exception No custom exceptions
std::string inet_ntoa(uint32_t addr);

// Highlights the parts of a string between delimiters with a color
// @param str The string to highlight
// @param color The color to use
// @param delim The delimiter characters
// @return The highlighted string
// @exception No custom exceptions
std::string highlight(std::string str, std::string color,
                      std::string delim = "'");

// Checks if a string contains a wildcard at the end
// @param str The string to check
// @return A vector of paths matching the wildcard
// @exception std::runtime_error if a function call fails
std::vector<std::string> processWildcard(std::string str);

// Decodes uri percent encoding
// @param str The string to decode
// @return The decoded string
// @exception std::runtime_error If the input uri is invalid
std::string uriDecode(std::string str);

// Encodes uri percent encoding
// @param str The string to encode
// @return The encoded string
// @exception No custom exceptions
std::string uriEncode(std::string str);

#endif