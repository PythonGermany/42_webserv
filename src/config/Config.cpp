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

std::string Config::getConfig() const { return _config; }

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
                              bool validateResult) {
  Log::write("Context: '" + context.getName() + "' -> Parsing", DEBUG);
  size_t startLine = line;
  while (true) {
    // Trim leading whitespace
    line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
    trimStart(data);
    if (data.length() == 0) break;

    // Find end of token
    size_t nextEnd = data.find_first_of(" \n");
    if (nextEnd == std::string::npos || data[nextEnd] != ' ')
      throwExeption(line, "Expected token ' ' not found");
    line += linesUntilPos(data, nextEnd);

    // Process token
    std::string token = trim(cut(data, 0, nextEnd));
    if (token == "include" || isValidDirective(context, token)) {
      // Find end of token value
      nextEnd = data.find_first_of(";\n");
      if (nextEnd == std::string::npos || data[nextEnd] != ';')
        throwExeption(line, "Expected token ';' not found");

      // Process token value
      if (token == "include")
        processInclude(context, trim(cut(data, 0, nextEnd)));
      else {
        checkError(line, validToAdd(context, token));
        std::vector<std::string> args =
            split(cut(data, 0, nextEnd), " \f\n\r\t\v");
        checkError(line, validArguments(context, token, args));
        context.addDirective(token, args);
      }
      data.erase(0, 1);
    } else if (isValidContext(context, token)) {
      processContext(context, data, token, line);
    } else
      checkError(line, "Invalid token '" + token + "'");
  }
  Log::write("Context: '" + context.getName() + "' -> Sucessfully parsed",
             DEBUG);
  // Validate parsed context
  if (validateResult) checkError(startLine, validate(context, false));
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
  checkError(line, validArguments(context, token, args));
  child.setArgs(args);

  line += linesUntilPos(data, data.find_first_not_of(" \f\t\v"));
  trimStart(data, " \f\t\v");

  // Find context end bracket
  size_t nextEnd = findContextEnd(data);
  if (nextEnd == std::string::npos)
    throwExeption(line, "No context end found for '" + token + "'");
  std::string contextData = cut(data, 1, nextEnd);

  // Parse context
  parseContext(child, contextData, line);
  checkError(line, validToAdd(context, token));
  context.addContext(child);
  line += linesUntilPos(contextData, contextData.length() + 2);
  data.erase(0, 2);
}

void Config::processInclude(Context &context, std::string path) {
  std::vector<std::string> files;
  // Prepare wildcard path
  std::string includePath;
  if (path[0] == '/')
    includePath = path;
  else
    includePath = _file.getDir() + path;

  // Get file list
  files = processWildcard(includePath);
  for (size_t i = 0; i < files.size(); i++) {
    Log::write(
        "Context: '" + context.getName() + "' -> Include '" + files[i] + "'",
        DEBUG);
    // Recursively parse included config files
    Config config(files[i]);
    config.removeComments();
    config.parseContext(context, config.getConfig(), 1, false);
  }
}

bool Config::isValidContext(Context &context, std::string token) const {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].parent == context.getName() &&
        tokens[i].isContext)
      return true;
  return false;
}

bool Config::isValidDirective(Context &context, std::string token) const {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].parent == context.getName() &&
        !tokens[i].isContext)
      return true;
  return false;
}

std::string Config::validToAdd(Context &context, std::string token) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++)
    if (tokens[i].name == token && tokens[i].parent == context.getName())
      return context.getTokenOccurence(token) < tokens[i].maxOccurence
                 ? ""
                 : "Token '" + token + "'has too many occurences";
  return "Token '" + token + "' not found";
}

std::string Config::validArguments(Context &context, std::string token,
                                   std::vector<std::string> args) {
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++) {
    if (tokens[i].name == token && tokens[i].parent == context.getName()) {
      if (tokens[i].func != NULL) {
        for (size_t j = 0; j < args.size(); j++) {
          std::string error = tokens[i].func(args[j], j);
          if (error != "") return "Argument '" + args[j] + "': " + error;
        }
      }
      if (tokens[i].minArgs <= args.size() && tokens[i].maxArgs >= args.size())
        return "";
      return "'" + token + "' requires between " + toString(tokens[i].minArgs) +
             " and " + toString(tokens[i].maxArgs) + " arguments";
    }
  }
  return "Token '" + token + "' not found";
}

std::string Config::validate(Context &context, bool recursive) {
  Log::write("Context: '" + context.getName() + "' -> Validating", DEBUG);
  for (size_t i = 0; i < sizeof(tokens) / sizeof(t_token); i++) {
    if (tokens[i].parent == context.getName() &&
        context.getTokenOccurence(tokens[i].name) < tokens[i].minOccurence) {
      if (tokens[i].isContext)
        return "Context missing required context '" + tokens[i].name + "'";
      return "Context missing required directive '" + tokens[i].name + "'";
    }
  }
  if (recursive) {
    std::map<std::string, std::vector<Context> >::iterator it;
    for (it = context.getContexts().begin(); it != context.getContexts().end();
         it++)
      for (size_t i = 0; i < it->second.size(); i++) validate(context, true);
  }
  Log::write("Context: '" + context.getName() + "' -> Sucessfully validated",
             DEBUG);
  return "";
}

int Config::linesUntilPos(const std::string &data, size_t pos) const {
  if (pos == std::string::npos) pos = data.length();
  int lines = 0;
  size_t i = data.find_first_of("\n");
  while (i < pos) {
    lines++;
    i = data.find_first_of("\n", i + 1);
  }
  return lines;
}

size_t Config::findContextEnd(const std::string &context) const {
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

void Config::checkError(size_t line, std::string error) {
  if (error != "") throwExeption(line, error);
}

void Config::throwExeption(size_t line, std::string msg) {
  throw std::runtime_error(_file.getPath() + ":" + toString(line) + ": " + msg);
}