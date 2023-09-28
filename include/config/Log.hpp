#ifndef LOG_HPP
#define LOG_HPP

#include <fstream>
#include <iostream>
#include <string>

#include "File.hpp"
#include "colors.hpp"
#include "utils.hpp"
#include "webserv.hpp"

class Log {
 private:
  static Log instance;

  static bool _log_to_stdout;
  static t_log_level _log_level;
  std::ofstream _log_file;
  std::ofstream _error_log_file;

  static std::string _timeFormat;
  static std::string _dateFormat;

  bool _customized;

  Log();

 public:
  ~Log();

  // Setters
  static void setLogToStdout(bool log, bool overwrite = false);
  static void setLevel(t_log_level level, bool overwrite = false);
  static void setLogFile(std::string path);
  static void setErrorLogFile(std::string path);

  // Getters
  static t_log_level getLevel();

  // Writes a message to the log file and if enabled also to the standard output
  // @exception No custom exceptions
  static void write(std::string msg, t_log_level level,
                    std::string color = RESET);
  // Writes a message to the error log file and to the standard error output
  // @exception No custom exceptions
  static void writeError(std::string msg, std::string color = RESET);
};

#endif