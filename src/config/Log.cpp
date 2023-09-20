#include "Log.hpp"

t_log_level Log::_log_level = LOG_LEVEL;
File Log::_log_file = File(LOG_PATH);
File Log::_error_log_file = File(ERROR_LOG_PATH);
std::string Log::_timeFormat = LOG_TIME_FORMAT;
std::string Log::_dateFormat = LOG_DATE_FORMAT;

Log::Log() {}

Log::~Log() {}

void Log::setLevel(t_log_level level) { _log_level = level; }

void Log::setLogFile(File path) { _log_file = path; }

void Log::setErrorLogFile(File path) { _error_log_file = path; }

t_log_level Log::getLevel() { return _log_level; }

File Log::getLogFile() { return _log_file; }

File Log::getErrorLogFile() { return _error_log_file; }

void Log::init() {
  try {
    if (!_log_file.exists()) _log_file.create();
    _log_file.open(O_WRONLY | O_APPEND);
  } catch (const std::exception& e) {
    std::cerr << "[" << getDate(_timeFormat) << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
  try {
    if (!_error_log_file.exists()) _error_log_file.create();
    _error_log_file.open(O_WRONLY | O_APPEND);
  } catch (const std::exception& e) {
    std::cerr << "[" << getDate(_timeFormat) << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
}

void Log::close() {
  try {
    _log_file.close();
  } catch (const std::exception& e) {
    std::cerr << "[" << getDate(_timeFormat) << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
  try {
    _error_log_file.close();
  } catch (const std::exception& e) {
    std::cerr << "[" << getDate(_timeFormat) << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
}

void Log::write(std::string msg, t_log_level level, std::string color) {
  if (level <= _log_level) {
    std::cout << "[" << getDate(_timeFormat) << "] " << color
              << highlight(msg, BRIGHT_BLUE) << RESET << std::endl;
    try {
      if (_log_file.isOpen())
        _log_file.write("[" + getDate(_dateFormat + " " + _timeFormat) + "] " +
                        msg + "\n");
    } catch (const std::exception& e) {
      std::cerr << "[" << getDate(_timeFormat) << "] " << BRIGHT_RED
                << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
    }
  }
}

void Log::writeError(std::string msg, std::string color) {
  std::cerr << "[" << getDate(_timeFormat) << "] " << BRIGHT_RED
            << "ERROR: " << color << msg << RESET << std::endl;
  try {
    if (_error_log_file.isOpen())
      _error_log_file.write("[" + getDate(_dateFormat + " " + _timeFormat) +
                            "] " + msg + "\n");
  } catch (const std::exception& e) {
    std::cerr << "[" << getDate(_timeFormat) << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
}
