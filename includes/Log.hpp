#ifndef LOG_HPP
#define LOG_HPP

#include <ctime>
#include <fstream>
#include <iostream>
#include <string>

#include "colors.hpp"
#include "webserv.hpp"

class Log {
 private:
  static t_log_level _log_level;
  static std::string _log_file;
  static std::string _error_log_file;

 public:
  Log();
  ~Log();

  // Setters
  static void setLogLevel(t_log_level level);
  static void setLogFile(std::string path);
  static void setErrorLogFile(std::string path);

  // Getters
  static t_log_level getLogLevel();
  static std::string getLogFile();
  static std::string getErrorLogFile();

  static void write(std::string msg, t_log_level level,
                    std::string color = RESET);
  static void writeError(std::string msg);
  static std::string getTimeStamp();
};

#endif