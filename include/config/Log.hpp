#ifndef LOG_HPP
#define LOG_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "File.hpp"
#include "colors.hpp"
#include "utils.hpp"

typedef enum log_level_e { ERROR, WARNING, INFO, DEBUG, VERBOSE } log_level_t;

typedef struct log_color_s {
  log_level_t level;
  std::string color;
} log_color_t;

// -------------------------- LOG VALUES ---------------------------

#define LOG_TO_STDOUT false
#define LOG_LEVEL INFO
#define LOG_STDOUT_OVERRIDE_LEVEL ERROR
#define LOG_PATH "/var/log/webserv/access.log"
#define LOG_ERROR_PATH "/var/log/webserv/error.log"
#define LOG_TIME_FORMAT "%H:%M:%S GMT"
#define LOG_DATE_FORMAT "%d-%m-%Y"

const log_color_t lvlColors[5] = {{ERROR, RED},
                                  {WARNING, YELLOW},
                                  {INFO, WHITE},
                                  {DEBUG, GRAY},
                                  {VERBOSE, GRAY}};

class Log {
 private:
  static bool _log_to_stdout;
  static log_level_t _log_level;

  std::string _path;
  std::ofstream _file;

  static std::string _timeFormat;
  static std::string _dateFormat;

  bool _fileSet;      // True once file has been set
  bool _initialized;  // Must be set to true in order to write to the log file

  bool _error;  // True if an error occured while writing to the log file

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

 private:
  std::string getLevelColor(log_level_t level);
};

#endif