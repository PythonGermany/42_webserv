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

std::string concatenate(const std::vector<std::string>& values,
                        std::string separator) {
  std::string value;
  for (size_t i = 0; i < values.size(); i++) {
    if (i != 0) value += separator;
    value += values[i];
  }
  return value;
}

bool startsWith(std::string str, std::string prefix) {
  if (str.size() < prefix.size()) return false;
  return !str.compare(0, prefix.size(), prefix);
}

bool endsWith(std::string str, std::string suffix) {
  if (str.size() < suffix.size()) return false;
  return !str.compare(str.size() - suffix.size(), suffix.size(), suffix);
}

std::string toHexString(unsigned char c) {
  std::string hex = "";
  hex += "0123456789ABCDEF"[c / 16];
  hex += "0123456789ABCDEF"[c % 16];
  return hex;
}

std::set<std::string> processWildcard(std::string path) {
  std::list<std::string> fs(1, path);
  std::set<std::string> ret;

  std::list<std::string>::iterator itr = fs.begin();
  for (; itr != fs.end(); itr++) {
    size_t wildcard = itr->find('*');
    if (wildcard == std::string::npos) {
      ret.insert(*itr);
      continue;
    }

    // Find pattern delimiters
    size_t patternStart = itr->rfind('/', wildcard);
    if (patternStart == std::string::npos)
      patternStart = 0;
    else
      patternStart += 1;
    size_t patternEnd = itr->find('/', wildcard);
    if (patternEnd == std::string::npos) patternEnd = itr->size();

    // Split input path
    std::string path = itr->substr(0, patternStart);
    std::string pattern = itr->substr(patternStart, patternEnd - patternStart);
    std::string prefix = itr->substr(patternEnd);
    fs.erase(itr--);

    // Prepare directory
    DIR* dir = opendir(path.c_str());
    if (dir == NULL) continue;
    struct dirent* ent;
    errno = 0;

    // Read directory
    while (true) {
      ent = readdir(dir);
      if (ent == NULL) break;
      if (ent->d_name == std::string(".") || ent->d_name == std::string(".."))
        continue;
      if (ent->d_type != DT_DIR &&
          prefix.find_first_of("*/") != std::string::npos)
        continue;
      int ret = fnmatch(pattern.c_str(), ent->d_name, 0);
      if (ret == 0) {
        fs.insert(itr, path + ent->d_name + prefix);
      } else if (ret != FNM_NOMATCH)
        throw std::runtime_error("fnmatch: " + std::string(strerror(errno)));
    }

    // Cleanup directory
    if (errno != 0)
      throw std::runtime_error("readdir: " + std::string(strerror(errno)));
    if (closedir(dir) == -1)
      throw std::runtime_error("closedir: " + std::string(strerror(errno)));
  }
  return ret;
}

std::string percentDecode(std::string str) {
  std::string decoded = "";
  for (size_t i = 0; i < str.length(); i++) {
    if (str[i] == '%') {
      if (i + 2 >= str.length())
        throw std::runtime_error("uriDecode: invalid string");
      int value;
      if (std::sscanf(str.substr(i + 1, 2).c_str(), "%x", &value) == EOF)
        throw std::runtime_error("uriDecode: invalid format");
      decoded += (char)value;
      i += 2;
    } else
      decoded += str[i];
  }
  return decoded;
}

std::string percentEncode(std::string str, std::string reserved) {
  std::string encoded = "";
  for (size_t i = 0; i < str.length(); i++) {
    if (std::isalnum(str[i]) ||
        std::string("-_.!~*'()").find(str[i]) != std::string::npos ||
        reserved.find(str[i]) != std::string::npos) {
      encoded += str[i];
    } else {
      std::string hex = toHexString(str[i]);
      std::transform(hex.begin(), hex.end(), hex.begin(), ::toupper);
      encoded += "%" + hex;
    }
  }
  return encoded;
}

std::string getTime(std::string format, const time_t* timer) {
  struct tm* timeinfo;
  size_t size = 64;

  time_t rawtime;
  if (timer == NULL) {
    std::time(&rawtime);
    timer = &rawtime;
  }
  if (!endsWith(format, " GMT")) format += " GMT";
  while (true) {
    char buffer[size];
    timeinfo = std::gmtime(timer);
    if (std::strftime(buffer, size, format.c_str(), timeinfo) != 0)
      return std::string(buffer);
    if (size > 256) return "";
    size *= 2;
  }
}

std::string getcwd() {
  size_t size = 1024;

  while (true) {
    char buffer[size];
    if (getcwd(buffer, size) != NULL) return buffer;
    if (errno != ERANGE || size > PATH_MAX) return "";
    size += 1024;
  }
}

size_t getStreamBufferSize(std::istream& stream) {
  stream.seekg(0, std::ios::end);
  size_t size = stream.tellg();
  stream.seekg(0, std::ios::beg);
  return size;
}

size_t toBytes(std::string size, size_t baseFactor) {
  size_t factor = 1;
  if (endsWith(size, "k")) {
    size.erase(size.size() - 1, 1);
    factor = baseFactor;
  } else if (endsWith(size, "m")) {
    size.erase(size.size() - 1, 1);
    factor = baseFactor * baseFactor;
  }
  return fromString<size_t>(size) * factor;
}