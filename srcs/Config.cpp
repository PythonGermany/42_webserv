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
  if (!_file.exists()) throwExeption("setFile", "File does not exist");
  if (!_file.file()) throwExeption("setFile", "Path is not a file");
  if (!_file.readable()) throwExeption("setFile", "File is not readable");
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
                             std::string parent) {
  Context context(name, parent);
  Log::write("Context: '" + name + "' -> Parsing", DEBUG);
  data = trim(data);
  while (data.length() > 0) {
    std::string token = trim(cut(data, 0, findToken(data, " ")));
    if (context.isValidContext(token)) {
      data = trim(data);
      context.addContext(
          parseContext(cut(data, 1, findContextEnd(data)), token, name));
      data.erase(0, 2);
    } else if (context.isValidDirective(token)) {
      context.addDirective(token,
                           split(cut(data, 0, findToken(data, ";")), " "));
      data.erase(0, 1);
    } else
      throwExeption("parseContext",
                    "Unknown token '" + token + "' for context '" + name + "'");
    data = trim(data);
  }
  Log::write("Context: '" + name + "' -> Sucessfully parsed", DEBUG);
  context.validate(false);
  return context;
}

int Config::findContextEnd(const std::string &context) {
  int depth = 0;
  int i = -1;
  while (i < (int)context.length()) {
    i = context.find_first_of("{}", i + 1);
    if (i == -1)
      throwExeption("findContextEnd", "Expected token '{' or '}' not found");
    if (context[i] == '{')
      depth++;
    else if (context[i] == '}')
      depth--;
    if (depth == 0) return i;
  }
  throwExeption("findContextEnd", "No context end found");
  return -1;
}

int Config::findToken(const std::string &data, std::string token) {
  int i = data.find(token);
  if (i == -1)
    throwExeption("find_token", "Expected token '" + token + "' not found");
  return i;
}

void Config::throwExeption(std::string func, std::string msg) {
  throw std::runtime_error("Config: " + func + ": " + msg);
}