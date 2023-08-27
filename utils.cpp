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
  tokens.push_back(str);
  return tokens;
}

bool isNumeric(std::string str) {
  for (std::string::iterator it = str.begin(); it != str.end(); it++)
    if (!isdigit(*it)) return false;
  return true;
}

bool endsWith(std::string str, std::string suffix) {
  return str.size() >= suffix.size() &&
         str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
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

std::string inet_hostname(uint32_t addr) {
  struct sockaddr_in address;
  address.sin_addr.s_addr = addr;
  char hostname[NI_MAXHOST];
  int result = getnameinfo((struct sockaddr *)&address, sizeof(address),
                           hostname, NI_MAXHOST, NULL, 0, 0);
  if (result != 0) return inet_ntoa(addr);
  return std::string(hostname);
}