#include "Log.hpp"

Log::Log() {}

Log::~Log() {}

void Log::write(std::string msg, t_log_level level, std::string color) {
  if (level <= LOG_LEVEL) {
    std::cout << "[" << getTimeStamp() << "] " << color << msg << RESET
              << std::endl;
    std::ofstream log(LOG_PATH, std::ios::app);
    log << "[" << getTimeStamp() << "] " << msg << std::endl;
    log.close();
  }
}

void Log::writeError(std::string msg) {
  std::cerr << "[" << getTimeStamp() << "] " << RED << "Error: " << RESET << msg
            << std::endl;
  std::ofstream errorLog(ERROR_LOG_PATH, std::ios::app);
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