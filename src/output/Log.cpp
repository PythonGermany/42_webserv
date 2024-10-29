#include "Log.hpp"

#include <iostream>
#include <map>

#include "File.hpp"
#include "utils.hpp"

bool Log::_log_to_terminal = LOG_TO_TERMINAL;
log_level_t Log::_log_level = LOG_LEVEL;
std::string Log::_timeFormat = LOG_TIME_FORMAT;
std::string Log::_dateFormat = LOG_DATE_FORMAT;

Log::Log(std::ostream &terminal) : _terminal(terminal) {}

Log::~Log() {}

void Log::init(std::string path) {
  File(path).createDirPath();
  _path = path;

  if (_file.is_open()) _file.close();
  _file.open(path.c_str(), std::ios_base::app);
}

void Log::setLogToTerminal(bool log, bool overwrite) {
  static bool initialized = false;
  if (overwrite || initialized == false) {
    _log_to_terminal = log;
    initialized = true;
  }
}

void Log::setLevel(log_level_t level, bool overwrite) {
  static bool initialized = false;
  if (overwrite || initialized == false) {
    _log_level = level;
    initialized = true;
  }
}

void Log::setFile(std::string path, bool overwrite) {
  if (overwrite || _fileSet == false) {
    File(path).createDirPath();
    _path = path;
    if (_file.is_open()) _file.close();
    _file.open(path.c_str(), std::ios_base::app);
    if (_file.good()) _logFileWriteError = false;
    _fileSet = true;
  }
}

void Log::setLogToFile(bool logToFile) { _logToFile = logToFile; }

log_level_t Log::getLevel() { return _log_level; }

static std::string getLevelString(log_level_t level) {
  switch (level) {
    case ERROR:
      return "Error  ";
    case WARNING:
      return "Warning";
    case INFO:
      return "Info   ";
    case DEBUG:
      return "Debug  ";
    case VERBOSE:
      return "Verbose";

    default:
      return "Unknown";
  }
}

bool Log::getLogToFile() { return _logToFile; }

void Log::write(std::string const &msg, log_level_t level, std::string color) {
  if (level > _log_level) return;
  std::string timeStamp = "[" + getTime(_dateFormat + " " + _timeFormat) + "]";
  std::string levelString = "[" + getLevelString(level) + "] ";
  if (_log_to_terminal) {
    if (color == RESET) color = getLevelColor(level);
    _terminal << timeStamp << color << levelString << msg << RESET << std::endl;
  }
  if (getLogToFile() == false) return;
  _file << timeStamp << levelString << msg << std::endl;
  if (_file.good() || _logFileWriteError == true) return;
  std::cerr << timeStamp << BRIGHT_RED << "ERROR: " << RESET
            << "Unable to write to log file: " << _path << std::endl;
  _logFileWriteError = true;
}

std::string Log::getLevelColor(log_level_t level) {
  static std::map<log_level_t, std::string> colors;

  std::map<log_level_t, std::string>::const_iterator itr = colors.find(level);
  if (itr != colors.end()) return itr->second;

  for (size_t i = 0; i < sizeof(lvlColors) / sizeof(log_color_t); i++) {
    if (lvlColors[i].level == level) {
      colors[level] = lvlColors[i].color;
      return lvlColors[i].color;
    }
  }
  return RESET;
}
