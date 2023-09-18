#include "Log.hpp"

t_log_level Log::_log_level = LOG_LEVEL;

File Log::_log_file = File(LOG_PATH);

File Log::_error_log_file = File(ERROR_LOG_PATH);

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
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
  try {
    if (!_error_log_file.exists()) _error_log_file.create();
    _error_log_file.open(O_WRONLY | O_APPEND);
  } catch (const std::exception& e) {
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
}

void Log::close() {
  try {
    _log_file.close();
  } catch (const std::exception& e) {
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
  try {
    _error_log_file.close();
  } catch (const std::exception& e) {
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
  }
}

void Log::write(std::string msg, t_log_level level, std::string color) {
  if (level <= _log_level) {
    std::cout << "[" << getTimeStamp() << "] " << color
              << highlight(msg, BRIGHT_BLUE) << RESET << std::endl;
    try {
      if (_log_file.isOpen())
        _log_file.write("[" + getDate() + " | " + getTimeStamp() + "] " + msg +
                        "\n");
    } catch (const std::exception& e) {
      std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
                << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
    }
  }
}

void Log::writeError(std::string msg, std::string color) {
  std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED << "ERROR: " << color
            << msg << RESET << std::endl;
  try {
    if (_error_log_file.isOpen())
      _error_log_file.write("[" + getDate() + " | " + getTimeStamp() + "] " +
                            msg + "\n");
  } catch (const std::exception& e) {
    std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED
              << "ERROR:" << RESET << " Log: " << e.what() << std::endl;
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

std::string Log::getDate() {
  time_t rawtime;
  struct tm* timeinfo;
  char buffer[11];

  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  std::strftime(buffer, 11, "%d-%m-%Y", timeinfo);
  return std::string(buffer);
}