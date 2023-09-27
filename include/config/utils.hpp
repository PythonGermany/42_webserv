#ifndef UTILS_HPP
#define UTILS_HPP

#include <dirent.h>
#include <fnmatch.h>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <list>
#include <sstream>
#include <vector>

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

// Concatenates a vector of strings into a single string
// @param values The vector of strings
// @param separator The separator to use
// @return The concatenated string
// @exception No custom exceptions
std::string concatenate(const std::vector<std::string>& values,
                        std::string separator);

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

// Converts a string to a value
// @param str The string to convert
// @return The value
// @exception No custom exceptions
template <typename T>
T fromString(std::string str) {
  std::stringstream ss(str);
  T val;
  ss >> val;
  return val;
}

// Converts a character to a hex string
std::string toHexString(unsigned char c);

// Highlights the parts of a string between delimiters with a color
// @param str The string to highlight
// @param color The color to use
// @param delim The delimiter characters
// @return The highlighted string
// @exception No custom exceptions
std::string highlight(std::string str, std::string color,
                      std::string delim = "'");

// Expands all the wildcards in a path into a list of fully expanded paths
// matching the wildcard patterns of the path
// @param path The full path with wildcard patterns
// @return A list of paths matching the wildcard path
// @exception std::runtime_error if a function call fails
std::list<std::string> processWildcard(std::string path);

// Decodes percent encoding
// @param str The string to decode
// @return The decoded string
// @exception std::runtime_error If the input uri is invalid
std::string percentDecode(std::string str);

// Encodes percent encoding
// @param str The string to encode
// @param reserved The reserved characters to encode
// @return The encoded string
// @exception No custom exceptions
std::string percentEncode(std::string str, std::string reserved);

// Returns the current date as a string
// @exception No custom exceptions
std::string getTime(std::string format, const time_t* timer = NULL);

// Returns a string describing the size in different formats depending on the
// size
std::string getMemorySize(size_t size);

// Returns the size until the end of the stream
size_t getStreamBufferSize(std::istream& stream);

#endif