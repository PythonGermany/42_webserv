#include "Log.hpp"

Log Log::instance;

bool Log::_log_to_stdout = LOG_TO_STDOUT;
t_log_level Log::_log_level = LOG_LEVEL;
std::string Log::_timeFormat = LOG_TIME_FORMAT;
std::string Log::_dateFormat = LOG_DATE_FORMAT;

Log::Log() : _log_file(LOG_PATH), _error_log_file(LOG_ERROR_PATH) {
  std::cout << "Default constructor called" << std::endl;  // TODO: remove
  std::cout << _log_file.is_open() << std::endl;           // TODO: remove
}

Log::~Log() {}

void Log::setLogToStdout(bool log) { _log_to_stdout = log; }

void Log::setLevel(t_log_level level) { _log_level = level; }

void Log::setLogFile(std::string path) {
  instance._log_file.open(path.c_str());
}

void Log::setErrorLogFile(std::string path) {
  instance._error_log_file.open(path.c_str());
}

t_log_level Log::getLevel() { return instance._log_level; }

void Log::write(std::string msg, t_log_level level, std::string color) {
  static bool error = false;
  if (level <= instance._log_level) {
    std::string timeStamp =
        "[" + getTime(_dateFormat + " " + _timeFormat) + "] ";
    if (instance._log_file.is_open())
      instance._log_file << timeStamp << msg << std::endl;
    else if (error == false) {
      error = true;
      std::cerr << BRIGHT_RED << "ERROR: " << RESET
                << "Unable to write to log file" << std::endl;
    }
    if (instance._log_to_stdout)
      std::cout << timeStamp << color << highlight(msg, BRIGHT_BLUE) << RESET
                << std::endl;
  }
}

void Log::writeError(std::string msg, std::string color) {
  static bool error = false;
  std::string timeStamp = "[" + getTime(_dateFormat + " " + _timeFormat) + "] ";
  if (instance._error_log_file.is_open())
    instance._error_log_file << timeStamp << msg << std::endl;
  else if (error == false) {
    std::cerr << BRIGHT_RED << "ERROR: " << RESET
              << "Unable to write to error log file" << std::endl;
  }
  std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << color << msg << RESET
            << std::endl;
}
