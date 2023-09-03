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
  Log::write(_file.getPath() + " Removing comments", DEBUG);
  size_t i = _config.find_first_of("#");
  while (i != std::string::npos) {
    size_t j = _config.find_first_of("\n", i);
    if (j == std::string::npos) j = _config.length();
    _config.erase(i, j - i);
    i = _config.find_first_of("#");
  }
  Log::write(_file.getPath() + " Comments removed", DEBUG);
}

Context &Config::parseContext(Context &context, std::string data, size_t line,
                              bool validate) {
  size_t startLine = line;
  std::string error;
  Log::write("Context: '" + context.getName() + "' -> Parsing", DEBUG);
  line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
  trimStart(data);
  while (data.length() > 0) {
    size_t nextEnd = data.find_first_of(" \n");
    if (nextEnd == std::string::npos || data[nextEnd] != ' ')
      throwExeption(line, "Expected token ' ' not found");
    line += linesUntilPos(data, nextEnd);
    std::string token = trim(cut(data, 0, nextEnd));
    if (token == "include" || context.isValidDirective(token)) {
      nextEnd = data.find_first_of(";\n");
      if (nextEnd == std::string::npos || data[nextEnd] != ';')
        throwExeption(line, "Expected token ';' not found");
      line += linesUntilPos(data, nextEnd + 1);
    }
    if (token == "include") {
      processInclude(context, trim(cut(data, 0, nextEnd)));
      data.erase(0, 1);
    } else if (context.isValidContext(token)) {
      line += linesUntilPos(data, data.find_first_not_of(" \f\t\v"));
      trimStart(data, " \f\t\v");
      if (data[0] != '{') throwExeption(line, "Expected token '{' not found");
      nextEnd = findContextEnd(data);
      if (nextEnd == std::string::npos)
        throwExeption(line, "No context end found for '" + token + "'");
      std::string contextData = cut(data, 1, nextEnd);
      Context child(token, context.getName());
      error = context.addContext(parseContext(child, contextData, line));
      line += linesUntilPos(contextData, contextData.length() + 2);
      data.erase(0, 2);
    } else if (context.isValidDirective(token)) {
      error = context.addDirective(token,
                                   split(cut(data, 0, nextEnd), " \f\n\r\t\v"));
      data.erase(0, 1);
    } else
      throwExeption(line, "Invalid token '" + token + "'");
    if (error != "") throwExeption(line, error);
    line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
    trimStart(data);
  }
  Log::write("Context: '" + context.getName() + "' -> Sucessfully parsed",
             DEBUG);
  if (validate) error = context.validate(false);
  if (error != "") throwExeption(startLine, error);
  return context;
}

void Config::processInclude(Context &context, std::string path) {
  std::vector<std::string> files = processWildcard(path);
  for (std::vector<std::string>::iterator it = files.begin(); it != files.end();
       it++) {
    Log::write(
        "Context: '" + context.getName() + "' -> Including '" + *it + "'",
        DEBUG);
    Config config(*it);
    config.removeComments();
    config.parseContext(context, config.getConfig(), 1, false);
  }
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
  throw std::runtime_error(_file.getPath() + ":" + toString(line) + ": " + msg);
}