#include "output.hpp"

int getBit(output_flag_t flag, int value) { return value & flag; }

void printInfo(int flags) {
  static bool isSet = false;
  if (getBit(SET, flags)) isSet = true;
  if (isSet && getBit(PRINT, flags)) {
    std::cout
        << BRIGHT_GREEN
        << "▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄\r\n"
           "██ ▄ ██ ▄ ███ ███ █ ▄▄█ ▄▄▀█ ▄▄█ ▄▄█ ▄▄▀█ ███ █\r\n"
           "█ ▀▀ ▀██▀▄███▄▀ ▀▄█ ▄▄█ ▄▄▀█▄▄▀█ ▄▄█ ▀▀▄██ ▀ ██\r\n"
           "████ ██ ▀▀████▄█▄██▄▄▄█▄▄▄▄█▄▄▄█▄▄▄█▄█▄▄███▄███\r\n"
           "\e[102m█ - An HTTP server with basic functionality - █\e[49m\r\n"
           "\e[102m█ https://github.com/PythonGermany/42_webserv █\e[49m\r\n"
           "▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀"
        << RESET << std::endl;
  }
  if (getBit(UNSET, flags)) isSet = false;
}

int printHelp(int flags) {
  static bool isSet = false;
  int ret = 0;
  if (getBit(SET, flags)) isSet = true;
  if (isSet && getBit(PRINT, flags)) {
    std::cout
        << BRIGHT_YELLOW << "Usage: " << RESET
        << "./webserv [configuration_file] [-i|-h|-v|-c|-t] [ [-s|-l|-a|-e "
           "ARGUMENT] ...]\r\n"
        << BRIGHT_YELLOW << "Flags:\r\n"
        << RESET
        << "  -i Show info block when starting the server\r\n"
           "  -h Show this help message and exit\r\n"
           "  -v Show version of webserv and exit\r\n"
           "  -c Show parsed config file structure and exit\r\n"
           "  -t Check if the config file syntax is valid and exit\r\n"
           "  -o Turn terminal output 'on' or 'off'\r\n"
           "  -l set the log level using '0/1/2/3' for "
           "error/warning/info/debug\r\n"
           "  -a Sets the path for the access log file\r\n"
           "  -e Sets the path for the error log file"
        << std::endl;
    ret = 1;
  }
  if (getBit(UNSET, flags)) isSet = false;
  return ret;
}

int printConfigStructure(int flags, std::string data) {
  static bool isSet = false;
  int ret = 0;
  if (getBit(SET, flags)) isSet = true;
  if (isSet && getBit(PRINT, flags)) {
    std::cout << data;
    ret = 1;
  }
  if (getBit(UNSET, flags)) isSet = false;
  return ret;
}

int printConfigValidation(int flags, std::string path) {
  static bool isSet = false;
  int ret = 0;
  if (getBit(SET, flags)) isSet = true;
  if (isSet && getBit(PRINT, flags)) {
    std::cout << BRIGHT_GREEN << "Configuration file " << path
              << " syntax is valid" << std::endl;
    ret = 1;
  }
  if (getBit(UNSET, flags)) isSet = false;
  return ret;
}

int printVersion(int flags) {
  static bool isSet = false;
  int ret = 0;
  if (getBit(SET, flags)) isSet = true;
  if (isSet && getBit(PRINT, flags)) {
    std::cout << BRIGHT_YELLOW << "webserv version: " << WEBSERV_ID << RESET
              << std::endl;
    ret = 1;
  }
  if (getBit(UNSET, flags)) isSet = false;
  return ret;
}