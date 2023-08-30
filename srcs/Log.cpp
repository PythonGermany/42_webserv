#include "Log.hpp"

t_log_level Log::_log_level = LOG_LEVEL;

File Log::_log_file = File(LOG_PATH);

File Log::_error_log_file = File(ERROR_LOG_PATH);

bool Log::_log_works = true;

bool Log::_error_log_works = true;

Log::Log() {}

Log::~Log() {}

void Log::setLogLevel(t_log_level level) { _log_level = level; }

void Log::setLogFile(File path) { _log_file = path; }

void Log::setErrorLogFile(File path) { _error_log_file = path; }

t_log_level Log::getLogLevel() { return _log_level; }

File Log::getLogFile() { return _log_file; }

File Log::getErrorLogFile() { return _error_log_file; }

void Log::createLogFiles() {
  try {
    if (!_log_file.exists()) _log_file.Create();
  } catch (const std::exception& e) {
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "Error:" << RESET << " Log: " << e.what() << std::endl;
    _log_works = false;
  }
  try {
    if (!_error_log_file.exists()) _error_log_file.Create();
  } catch (const std::exception& e) {
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "Error:" << RESET << " Log: " << e.what() << std::endl;
    _error_log_works = false;
  }
}

void Log::write(std::string msg, t_log_level level, std::string color) {
  if (level <= _log_level) {
    std::cout << "[" << getTimeStamp() << "] " << color
              << highlight(msg, BRIGHT_BLUE) << RESET << std::endl;
    try {
      if (_log_works)
        _log_file.Write("[" + getTimeStamp() + "] " + msg + "\n", true);
    } catch (const std::exception& e) {
      std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
                << "Error:" << RESET << " Log: " << e.what() << std::endl;
      _log_works = false;
    }
  }
}

void Log::writeError(std::string msg, std::string color) {
  std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED << "Error: " << color
            << msg << RESET << std::endl;
  try {
    if (_error_log_works)
      _error_log_file.Write("[" + getTimeStamp() + "] " + msg + "\n", true);
  } catch (const std::exception& e) {
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "Error:" << RESET << " Log: " << e.what() << std::endl;
    _error_log_works = false;
  }
}

std::string Log::getTimeStamp() {
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[9];

  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  std::strftime(buffer, 9, "%H:%M:%S", timeinfo);
  return std::string(buffer);
}