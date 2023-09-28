#include "Log.hpp"

Log Log::_instance;

bool Log::_log_to_stdout = LOG_TO_STDOUT;
t_log_level Log::_log_level = LOG_LEVEL;
std::string Log::_timeFormat = LOG_TIME_FORMAT;
std::string Log::_dateFormat = LOG_DATE_FORMAT;

Log::Log() {
  File(LOG_PATH).createDirPath();
  File(LOG_ERROR_PATH).createDirPath();
  _log_file.open(LOG_PATH, std::ios_base::app);
  _error_log_file.open(LOG_ERROR_PATH, std::ios_base::app);
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

void Log::setLogFile(std::string path, bool overwrite) {
  static bool initialized = false;
  if (overwrite || !initialized) {
    File(path).createDirPath();
    if (_instance._log_file.is_open()) _instance._log_file.close();
    _instance._log_file.open(path.c_str(), std::ios_base::app);
    initialized = true;
  }
}

void Log::setErrorLogFile(std::string path, bool overwrite) {
  static bool initialized = false;
  if (overwrite || !initialized) {
    File(path).createDirPath();
    if (_instance._error_log_file.is_open()) _instance._error_log_file.close();
    _instance._error_log_file.open(path.c_str(), std::ios_base::app);
    initialized = true;
  }
}

void Log::setAllowError(bool allow) { _instance._allowError = allow; }

t_log_level Log::getLevel() { return _instance._log_level; }

void Log::write(std::string msg, t_log_level level, std::string color) {
  static bool firstError = true;
  if (level <= _instance._log_level) {
    std::string timeStamp =
        "[" + getTime(_dateFormat + " " + _timeFormat) + "] ";
    if (_instance._log_to_stdout)
      std::cout << timeStamp << color << highlight(msg, BRIGHT_BLUE) << RESET
                << std::endl;
    _instance._log_file << timeStamp << msg << std::endl;
    if (!_instance._log_file.good() && _instance._allowError && firstError) {
      std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << RESET
                << "Unable to write to log file" << std::endl;
      firstError = false;
    }
  }
}

void Log::writeError(std::string msg, std::string color) {
  static bool firstError = true;
  std::string timeStamp = "[" + getTime(_dateFormat + " " + _timeFormat) + "] ";
  std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << color << msg << RESET
            << std::endl;
  _instance._error_log_file << timeStamp << msg << std::endl;
  if (!_instance._error_log_file.good() && _instance._allowError &&
      firstError) {
    std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << RESET
              << "Unable to write to error log file" << std::endl;
    firstError = false;
  }
}
