#ifndef UTILS_HPP
#define UTILS_HPP

#include <netdb.h>

#include <sstream>
#include <vector>

#include "colors.hpp"
#include "webserv.hpp"

std::string trim(const std::string& str, std::string chars = " \f\n\r\t\v");

std::string cut(std::string& str, int start, int end);

std::vector<std::string> split(const std::string& str, std::string delim);

bool startsWith(std::string str, std::string suffix);

bool endsWith(std::string str, std::string suffix);

template <typename T>
std::string toString(T val) {
  std::stringstream ss;
  ss << val;
  return ss.str();
}

std::string inet_ntoa(uint32_t addr);

std::string highlight(std::string str, std::string color,
                      std::string delim = "'");

#endif