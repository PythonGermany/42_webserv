#ifndef LOG_HPP
#define LOG_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <ostream>
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

#define LOG_TO_TERMINAL false
#define LOG_LEVEL INFO
#define LOG_TERMINAL std::cout
#define LOG_PATH "/var/log/webserv/access.log"
#define LOG_ERROR_TERMINAL std::cerr
#define LOG_ERROR_PATH "/var/log/webserv/error.log"
#define LOG_TIME_FORMAT "%H:%M:%S GMT"
#define LOG_DATE_FORMAT "%d-%m-%Y"

const log_color_t lvlColors[5] = {{ERROR, BRIGHT_RED},
                                  {WARNING, YELLOW},
                                  {INFO, WHITE},
                                  {DEBUG, GRAY},
                                  {VERBOSE, GRAY}};

class Log {
 private:
  static bool _log_to_terminal;
  static log_level_t _log_level;

  std::string _path;
  std::ostream& _terminal;
  std::ofstream _file;

  static std::string _timeFormat;
  static std::string _dateFormat;

  bool _fileSet;      // True once file has been set
  bool _initialized;  // Enables/Disables logfile functionality

  bool _error;  // True if an error occured while writing to the log file

 public:
  Log(std::ostream& terminal);
  ~Log();

  void init(std::string path);

  // Setters
  static void setLogToTerminal(bool log, bool overwrite = false);
  static void setLevel(log_level_t level, bool overwrite = false);
  void setFile(std::string path, bool overwrite = false);
  void setInitialized(bool intialized);

  // Getters
  static log_level_t getLevel();
  bool getInitialized();

  // Writes a message to the log file and if enabled also to the _terminal
  // stream
  // @exception No custom exceptions
  void write(std::string const& msg, log_level_t level,
             std::string color = RESET);

 private:
  std::string getLevelColor(log_level_t level);
};

#endif
