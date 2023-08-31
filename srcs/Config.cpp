#include "Config.hpp"

Config::Config() {}

Config::Config(std::string path) { setFile(path); }

Config::Config(const Config &rhs) { *this = rhs; }

Config &Config::operator=(const Config &rhs) {
  if (this == &rhs) return *this;
  _file = rhs._file;
  _config = rhs._config;
  return *this;
}

Config::~Config() {}

void Config::setFile(std::string path) {
  _file = File(path);
  if (!_file.exists()) throw std::runtime_error("Config: File not found");
  if (!_file.readable()) throw std::runtime_error("Config: File not readable");
  _file.open();
  _config = _file.read();
  _file.close();
}

std::string Config::getConfig() { return _config; }

void Config::removeComments() {
  Log::write("Config: Removing comments", DEBUG);
  size_t i = _config.find_first_of("#");
  while (i != std::string::npos) {
    size_t j = _config.find_first_of("\n", i);
    if (j == std::string::npos) j = _config.length();
    _config.erase(i, j - i);
    i = _config.find_first_of("#");
  }
  Log::write("Config: Comments removed", DEBUG);
}

Context Config::parseContext(std::string data, std::string name,
                             std::string parent, size_t line) {
  Context context(name, parent);
  size_t startLine = line;
  std::string error;
  Log::write("Context: '" + name + "' -> Parsing", DEBUG);
  line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
  data = trim(data);
  while (data.length() > 0) {
    size_t nextEnd = data.find_first_of(" \n");
    if (nextEnd == std::string::npos || data[nextEnd] != ' ')
      throwExeption(line, "Expected token ' ' not found");
    line += linesUntilPos(data, nextEnd);
    std::string token = trim(cut(data, 0, nextEnd));
    if (context.isValidContext(token)) {
      line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
      data = trim(data);
      size_t contextEnd = findContextEnd(data);
      if (contextEnd == std::string::npos)
        throwExeption(line, "No context end found for '" + token + "'");
      std::string contextData = cut(data, 1, contextEnd);
      error = context.addContext(parseContext(contextData, token, name, line));
      line += linesUntilPos(contextData, contextData.length() + 2);
      data.erase(0, 2);
    } else if (context.isValidDirective(token)) {
      nextEnd = data.find_first_of(";\n");
      if (nextEnd == std::string::npos || data[nextEnd] != ';')
        throwExeption(line, "Expected token ';' not found");
      line += linesUntilPos(data, nextEnd + 1);
      error = context.addDirective(token,
                                   split(cut(data, 0, nextEnd), " \f\n\r\t\v"));
      data.erase(0, 1);
    } else
      throwExeption(line, "Invalid token '" + token + "'");
    if (error != "") throwExeption(line, error);
    line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
    data = trim(data);
  }
  Log::write("Context: '" + name + "' -> Sucessfully parsed", DEBUG);
  error = context.validate(false);
  if (error != "") throwExeption(startLine, "Context '" + name + "': " + error);
  return context;
}

int Config::linesUntilPos(const std::string &data, size_t pos) {
  if (pos == std::string::npos) pos = data.length();
  int lines = 0;
  size_t i = data.find_first_of("\n", 0);
  while (i < pos) {
    lines++;
    i = data.find_first_of("\n", i + 1);
  }
  return lines;
}

size_t Config::findContextEnd(const std::string &context) {
  int depth = 0;
  int i = -1;
  while (i < (int)context.length()) {
    i = context.find_first_of("{}", i + 1);
    if (i == -1) return std::string::npos;
    if (context[i] == '{')
      depth++;
    else if (context[i] == '}')
      depth--;
    if (depth == 0) return i;
  }
  return std::string::npos;
}

void Config::throwExeption(size_t line, std::string msg) {
  throw std::runtime_error("Config: Line " + toString(line) + ": " + msg);
}