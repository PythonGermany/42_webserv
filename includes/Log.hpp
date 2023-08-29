#ifndef LOG_HPP
#define LOG_HPP

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "colors.hpp"
#include "webserv.hpp"

class Log {
 public:
  Log();
  ~Log();

 public:
  static void write(std::string msg, t_log_level level,
                    std::string color = RESET);
  static void writeError(std::string msg);
  static std::string getTimeStamp();
};

#endif