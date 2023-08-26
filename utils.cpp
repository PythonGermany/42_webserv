#include "utils.hpp"

std::string trim(std::string str) {
  size_t first = str.find_first_not_of(" \f\n\r\t\v");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \f\n\r\t\v");
  return str.substr(first, (last - first + 1));
}

bool exists(std::string path) {
  struct stat buf;
  return stat(path.c_str(), &buf) == 0;
}

bool is_file(std::string path) {
  struct stat buf;
  stat(path.c_str(), &buf);
  return S_ISREG(buf.st_mode);
}

bool is_dir(std::string path) {
  struct stat buf;
  stat(path.c_str(), &buf);
  return S_ISDIR(buf.st_mode);
}

bool is_readable(std::string path) {
  struct stat buf;
  stat(path.c_str(), &buf);
  return buf.st_mode & S_IRUSR;
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