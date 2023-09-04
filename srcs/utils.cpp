#include "utils.hpp"

std::string trim(const std::string& str, std::string chars) {
  size_t first = str.find_first_not_of(chars);
  if (first == std::string::npos) return "";
  size_t last = str.find_last_not_of(chars);
  return str.substr(first, (last - first + 1));
}

std::string& trimStart(std::string& str, std::string chars) {
  size_t first = str.find_first_not_of(chars);
  if (first == std::string::npos) first = str.length();
  return str.erase(0, first);
}

std::string cut(std::string& str, int start, int end) {
  std::string cut = str.substr(start, end - start);
  str.erase(start, end - start);
  return cut;
}

std::vector<std::string> split(const std::string& str, std::string delim) {
  std::vector<std::string> tokens;
  size_t start = 0;
  size_t end = str.find_first_of(delim);
  while (end != std::string::npos) {
    if (start != end) tokens.push_back(str.substr(start, end - start));
    start = end + 1;
    end = str.find_first_of(delim, start);
  }
  if (start < str.length() && start != end)
    tokens.push_back(str.substr(start, end - start));
  return tokens;
}

bool startsWith(std::string str, std::string prefix) {
  if (str.size() < prefix.size()) return false;
  return !str.compare(0, prefix.size(), prefix);
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

// TODO: Error handling
std::vector<std::string> processWildcard(std::string str) {
  std::vector<std::string> files;
  std::string path = str.substr(0, str.find_last_of("/"));
  std::string pattern = str.substr(str.find_last_of("/") + 1);
  DIR* dir = opendir(path.c_str());
  if (dir == NULL) throw std::runtime_error("opendir() failed");
  struct dirent* ent;
  errno = 0;
  ent = readdir(dir);
  while (ent != NULL) {
    if (ent->d_name == std::string(".") || ent->d_name == std::string(".."))
      continue;
    int ret = fnmatch(pattern.c_str(), ent->d_name, 0);
    if (ret == FNM_NOMATCH)
      continue;
    else if (ret == 0)
      files.push_back(path + "/" + ent->d_name);
    else if (ret == FNM_NOSYS)
      throw std::runtime_error("fnmatch() failed");
    ent = readdir(dir);
  }
  if (errno != 0) throw std::runtime_error("readdir() failed");
  if (closedir(dir) == -1) throw std::runtime_error("closedir() failed");
  return files;
}