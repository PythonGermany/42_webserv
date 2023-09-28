#include "Log.hpp"

Log Log::instance;

bool Log::_log_to_stdout = LOG_TO_STDOUT;
t_log_level Log::_log_level = LOG_LEVEL;
std::string Log::_timeFormat = LOG_TIME_FORMAT;
std::string Log::_dateFormat = LOG_DATE_FORMAT;

Log::Log() {
  File(LOG_PATH).createDirPath();
  File(LOG_ERROR_PATH).createDirPath();
  _log_file.open(LOG_PATH, std::ios_base::app);
  _error_log_file.open(LOG_ERROR_PATH, std::ios_base::app);
  _customized = false;
}

Log::~Log() {}

void Log::setLogToStdout(bool log, bool overwrite) {
  static bool initialized = false;
  if (overwrite || !initialized) {
    _log_to_stdout = log;
    initialized = true;
  }
}

void Log::setLevel(t_log_level level, bool overwrite) {
  static bool initialized = false;
  if (overwrite || !initialized) {
    _log_level = level;
    initialized = true;
  }
}

void Log::setLogFile(std::string path) {
  File(path).createDirPath();
  instance._log_file.open(path.c_str(), std::ios_base::app);
  if (instance._log_file.fail()) instance._log_file.close();
  instance._customized = true;
}

void Log::setErrorLogFile(std::string path) {
  File(path).createDirPath();
  instance._error_log_file.open(path.c_str(), std::ios_base::app);
  if (instance._error_log_file.fail()) instance._error_log_file.close();
  instance._customized = true;
}

t_log_level Log::getLevel() { return instance._log_level; }

void Log::write(std::string msg, t_log_level level, std::string color) {
  static bool error = false;
  if (level <= instance._log_level) {
    std::string timeStamp =
        "[" + getTime(_dateFormat + " " + _timeFormat) + "] ";
    instance._log_file << timeStamp << msg << std::endl;
    if (instance._customized && !instance._log_file.good() && error == false) {
      error = true;
      writeError("Unable to write to log file", BRIGHT_YELLOW);
    }
    if (instance._log_to_stdout)
      std::cout << timeStamp << color << highlight(msg, BRIGHT_BLUE) << RESET
                << std::endl;
  }
}

void Log::writeError(std::string msg, std::string color) {
  static bool error = false;
  std::string timeStamp = "[" + getTime(_dateFormat + " " + _timeFormat) + "] ";
  instance._error_log_file << timeStamp << msg << std::endl;
  if (instance._customized && !instance._error_log_file.good() &&
      error == false) {
    std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << RESET
              << "Unable to write to error log file" << std::endl;
  }
  std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << color << msg << RESET
            << std::endl;
}
