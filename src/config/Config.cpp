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
  if (!_file.exists())
    throw std::runtime_error("Config: File '" + path + "' not found");
  if (!_file.readable())
    throw std::runtime_error("Config: File '" + path + "' not readable");
  _file.open(O_RDONLY);
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
  Log::write("Context: '" + context.getName() + "' -> Parsing", DEBUG);
  size_t startLine = line;
  while (true) {
    line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
    trimStart(data);
    if (data.length() == 0) break;
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
      if (token == "include")
        _error = processInclude(context, trim(cut(data, 0, nextEnd)));
      else
        _error = context.addDirective(
            token, split(cut(data, 0, nextEnd), " \f\n\r\t\v"));
      data.erase(0, 1);
    } else if (context.isValidContext(token)) {
      processContext(context, data, token, line);
    } else
      _error = "Invalid token '" + token + "'";
    if (_error != "") throwExeption(line, _error);
  }
  Log::write("Context: '" + context.getName() + "' -> Sucessfully parsed",
             DEBUG);
  if (validate) _error = context.validate(false);
  if (_error != "") throwExeption(startLine, _error);
  return context;
}

void Config::processContext(Context &context, std::string &data,
                            std::string token, size_t &line) {
  Context child(token, &context);
  // Find context start bracket
  size_t argsEnd = data.find_first_of("{\n");
  if (argsEnd == std::string::npos || data[argsEnd] != '{')
    throwExeption(line, "Expected token '{' not found");

  // Parse, validate and add
  std::vector<std::string> args =
      split(trim(cut(data, 0, argsEnd)), " \f\n\r\t\v");
  _error = child.isValidContextArgs(args);
  if (_error != "") throwExeption(line, _error);
  child.setArgs(args);

  trimStart(data, " \f\t\v");
  line += linesUntilPos(data, data.find_first_not_of(" \f\t\v"));

  // Find context end bracket
  size_t nextEnd = findContextEnd(data);
  if (nextEnd == std::string::npos)
    throwExeption(line, "No context end found for '" + token + "'");
  std::string contextData = cut(data, 1, nextEnd);

  // Parse context
  parseContext(child, contextData, line);
  _error = context.addContext(child);
  line += linesUntilPos(contextData, contextData.length() + 2);
  data.erase(0, 2);
}

std::string Config::processInclude(Context &context, std::string path) {
  std::vector<std::string> files;
  try {
    std::string includePath;
    if (path[0] == '/')
      includePath = path;
    else
      includePath = _file.getDir() + "/" + path;
    files = processWildcard(includePath);
  } catch (const std::exception &e) {
    return e.what();
  }
  for (size_t i = 0; i < files.size(); i++) {
    Log::write(
        "Context: '" + context.getName() + "' -> Include '" + files[i] + "'",
        DEBUG);
    Config config(files[i]);
    config.removeComments();
    config.parseContext(context, config.getConfig(), 1, false);
  }
  return "";
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