#ifndef LOG_HPP
#define LOG_HPP

#include <ctime>
#include <iostream>
#include <string>

#include "File.hpp"
#include "colors.hpp"
#include "utils.hpp"
#include "../webserv.hpp"

class Log {
 private:
  static t_log_level _log_level;
  static File _log_file;
  static File _error_log_file;

 public:
  Log();
  ~Log();

  // Setters
  static void setLevel(t_log_level level);
  static void setLogFile(File path);
  static void setErrorLogFile(File path);

  // Getters
  static t_log_level getLevel();
  static File getLogFile();
  static File getErrorLogFile();

  // This function should be called at the beginning of the program
  // It will create the log file and the error log file if they don't exist and
  // open them
  // @exception No custom exceptions
  static void init();
  // This function should be called at the end of the program
  // It will close the log file and the error log file
  // @exception No custom exceptions
  static void close();

  // Writes a message to the log file and to the standard output
  // @exception No custom exceptions
  static void write(std::string msg, t_log_level level,
                    std::string color = RESET);
  // Writes a message to the error log file and to the standard error output
  // @exception No custom exceptions
  static void writeError(std::string msg, std::string color = RESET);
  // Returns the current time stamp in the format [HH:MM:SS]
  // @exception No custom exceptions
  static std::string getTimeStamp();
  // Returns the current date in the format [DD/MM/YYYY]
  // @exception No custom exceptions
  static std::string getDate();
};

#endif