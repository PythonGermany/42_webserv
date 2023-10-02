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
  static bool _log_to_stdout;
  static log_level_t _log_level;

  std::string _path;
  std::ofstream _file;

  static std::string _timeFormat;
  static std::string _dateFormat;

  bool _initialized;

 public:
  Log(std::string path);
  ~Log();

  // Setters
  static void setLogToStdout(bool log, bool overwrite = false);
  static void setLevel(log_level_t level, bool overwrite = false);
  void setFile(std::string path, bool overwrite = false);
  void setInitialized(bool intialized);

  // Getters
  static log_level_t getLevel();

  // Writes a message to the log file and if enabled also to the standard output
  // @exception No custom exceptions
  void write(std::string msg, log_level_t level, std::string color = RESET);
};

#endif