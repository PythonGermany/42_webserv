#include "utils.hpp"

std::string trim(std::string str) {
  size_t first = str.find_first_not_of(" \f\n\r\t\v");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \f\n\r\t\v");
  return str.substr(first, (last - first + 1));
}

long int size(std::string path) {
  struct stat buf;
  stat(path.c_str(), &buf);
  return buf.st_size;
}

long int modified(std::string path) {
  struct stat buf;
  stat(path.c_str(), &buf);
  return buf.st_mtim.tv_sec;
}