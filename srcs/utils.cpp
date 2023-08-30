#include "utils.hpp"

std::string trim(std::string str) {
  size_t first = str.find_first_not_of(" \f\n\r\t\v");
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(" \f\n\r\t\v");
  return str.substr(first, (last - first + 1));
}

std::string cut(std::string &str, int start, int end) {
  std::string cut = str.substr(start, end - start);
  str = str.substr(0, start) + str.substr(end);
  return cut;
}

std::vector<std::string> split(std::string str, std::string delim) {
  std::vector<std::string> tokens;
  size_t pos = 0;
  std::string token;
  while ((pos = str.find(delim)) != std::string::npos) {
    if (pos != 0) tokens.push_back(str.substr(0, pos));
    str.erase(0, pos + delim.length());
  }
  if (str.length() > 0) tokens.push_back(str);
  return tokens;
}

bool startsWith(std::string str, std::string suffix) {
  if (str.size() < suffix.size()) return false;
  return !str.compare(0, suffix.size(), suffix);
}

bool endsWith(std::string str, std::string suffix) {
  if (str.size() < suffix.size()) return false;
  return !str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

std::string inet_ntoa(uint32_t addr) {
  std::string str = "";
  for (int i = 0; i < 4; i++) {
    str += toString((int)addr & 0xFF);
    if (i != 3) str += ".";
    addr >>= 8;
  }
  return str;
}

std::string highlight(std::string str, std::string color, std::string delim) {
  bool first = true;
  size_t i = str.find_first_of(delim);
  while (i != std::string::npos) {
    if (first == true) {
      str.insert(i, color);
      i += color.length();
    } else
      str.insert(i + delim.length(), RESET);
    first = !first;
    i = str.find_first_of(delim, i + delim.length());
  }
  return str;
}