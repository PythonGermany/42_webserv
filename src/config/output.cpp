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
    std::cout << BRIGHT_RED << "Usage: " << BRIGHT_GREEN
              << "./webserv [configuration_file] [-i|-h] [ [-FLAG "
                 "ARGUMENT] ...]\r\n"
              << BRIGHT_RED << "Flags:\r\n"
              << BRIGHT_GREEN
              << "  -i Prints out a info block when starting the server\r\n"
                 "  -h Prints this help message\r\n"
                 "  -s Turn stdout 'on' or 'off'\r\n"
                 "  -l set the log level using '0/1/2/3' for "
                 "error/warning/info/debug\r\n"
                 "  -a Sets the path for the access log file\r\n"
                 "  -e Sets the path for the error log file"
              << RESET << std::endl;
    ret = 1;
  }
  if (getBit(UNSET, flags)) isSet = false;
  return ret;
}