#include "Log.hpp"

t_log_level Log::_log_level = LOG_LEVEL;

std::string Log::_log_file = LOG_PATH;

std::string Log::_error_log_file = ERROR_LOG_PATH;

Log::Log() {}

Log::~Log() {}

void Log::setLogLevel(t_log_level level) { _log_level = level; }

void Log::setLogFile(std::string path) { _log_file = path; }

void Log::setErrorLogFile(std::string path) { _error_log_file = path; }

t_log_level Log::getLogLevel() { return _log_level; }

std::string Log::getLogFile() { return _log_file; }

std::string Log::getErrorLogFile() { return _error_log_file; }

void Log::write(std::string msg, t_log_level level, std::string color) {
  if (level <= _log_level) {
    std::string terminal = highlight(msg, BRIGHT_BLUE);
    std::cout << "[" << getTimeStamp() << "] " << color << terminal << RESET
              << std::endl;
    std::ofstream log(_log_file.c_str(), std::ios::app);
    log << "[" << getTimeStamp() << "] " << msg << std::endl;
    log.close();
  }
}

void Log::writeError(std::string msg, std::string color) {
  std::cerr << "[" << getTimeStamp() << "] " << BRIGHT_RED << "Error: " << color
            << msg << RESET << std::endl;
  std::ofstream errorLog(_error_log_file.c_str(), std::ios::app);
  errorLog << "[" << getTimeStamp() << "] "
           << "Error: " << msg << std::endl;
  errorLog.close();
}

std::string Log::getTimeStamp() {
  time_t rawtime;
  struct tm *timeinfo;
  char buffer[9];

  std::time(&rawtime);
  timeinfo = std::localtime(&rawtime);

  std::strftime(buffer, 9, "%H:%M:%S", timeinfo);
  return std::string(buffer);
}