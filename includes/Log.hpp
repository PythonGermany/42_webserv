#ifndef LOG_HPP
#define LOG_HPP

#include <ctime>
#include <iostream>
#include <string>

#include "File.hpp"
#include "colors.hpp"
#include "utils.hpp"
#include "webserv.hpp"

class Log {
 private:
  static t_log_level _log_level;
  static File _log_file;
  static File _error_log_file;
  static bool _log_works;
  static bool _error_log_works;

 public:
  Log();
  ~Log();

  // Setters
  static void setLogLevel(t_log_level level);
  static void setLogFile(File path);
  static void setErrorLogFile(File path);

  // Getters
  static t_log_level getLogLevel();
  static File getLogFile();
  static File getErrorLogFile();

  static void write(std::string msg, t_log_level level,
                    std::string color = RESET);
  static void writeError(std::string msg, std::string color = RESET);
  static std::string getTimeStamp();
};

#endif