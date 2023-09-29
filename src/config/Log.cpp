#include "Log.hpp"

bool Log::_log_to_stdout = LOG_TO_STDOUT;
log_level_t Log::_log_level = LOG_LEVEL;
std::string Log::_timeFormat = LOG_TIME_FORMAT;
std::string Log::_dateFormat = LOG_DATE_FORMAT;
bool Log::_allowError = false;

Log::Log(std::string path) {
  File(path).createDirPath();
  _path = path;
  _file.open(path.c_str(), std::ios_base::app);
}

Log::~Log() {}

void Log::setLogToStdout(bool log, bool overwrite) {
  static bool initialized = false;
  if (overwrite || !initialized) {
    _log_to_stdout = log;
    initialized = true;
  }
}

void Log::setLevel(log_level_t level, bool overwrite) {
  static bool initialized = false;
  if (overwrite || !initialized) {
    _log_level = level;
    initialized = true;
  }
}

void Log::setFile(std::string path, bool overwrite) {
  if (overwrite || !_initialized) {
    File(path).createDirPath();
    _path = path;
    if (_file.is_open()) _file.close();
    _file.open(path.c_str(), std::ios_base::app);
    _initialized = true;
  }
}

void Log::setAllowError(bool allow) { _allowError = allow; }

log_level_t Log::getLevel() { return _log_level; }

void Log::write(std::string msg, log_level_t level, std::string color) {
  static bool firstError = true;
  if (level <= _log_level) {
    std::string timeStamp =
        "[" + getTime(_dateFormat + " " + _timeFormat) + "] ";
    if (_log_to_stdout || level == ERROR)
      std::cout << timeStamp << color << highlight(msg, BRIGHT_BLUE) << RESET
                << std::endl;
    _file << timeStamp << msg << std::endl;
    if (!_file.good() && _allowError && firstError) {
      std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << RESET
                << "Unable to write to log file: " << _path << std::endl;
      firstError = false;
    }
  }
}