#ifndef UTILS_HPP
#define UTILS_HPP

#include <netdb.h>
#include <sys/stat.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "webserv.hpp"

std::string trim(std::string str);

std::string cut(std::string &str, int start, int end);

std::vector<std::string> split(std::string str, std::string delim);

bool isNumeric(std::string str);

bool startsWith(std::string str, std::string suffix);

bool endsWith(std::string str, std::string suffix);

template <typename T>
std::string toString(T val) {
  std::stringstream ss;
  ss << val;
  return ss.str();
}

std::string inet_ntoa(uint32_t addr);

std::string getTimeStamp();

void writeToLog(std::string msg, t_log_level level);

void writeToErrorLog(std::string msg);

#endif