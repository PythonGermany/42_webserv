#include "Config.hpp"

std::string isMimeType(std::string const &value, size_t index) {
  if (value.length() == 0) return "Mime type cannot be empty";
  if (index == 0) {
    if (value.find('/') == std::string::npos)
      return "Mime type must contain a '/'";
  } else
    return isExtension(value, index);
  return "";
}

std::string isErrorPage(std::string const &value, size_t index) {
  if (index == 0) {
    for (size_t i = 0; i < sizeof(codes_g) / sizeof(codes_g[0]); i++)
      if (value == codes_g[i]) return "";
    return "Invalid error code";
  } else
    return isAbsolutePath(value, index);
}

std::string isNumeric(std::string const &value, size_t index) {
  (void)index;
  for (size_t i = 0; i < value.length(); i++)
    if (std::isdigit(value[i]) == false)
      return "All characters must be numeric";
  return "";
}

std::string isMemorySize(std::string const &value, size_t index) {
  std::string v = value;
  if (endsWith(value, "k") || endsWith(value, "m"))
    v = value.substr(0, value.size() - 1);
  if (isNumeric(v, index) != "")
    return "All characters must be numeric exept the last one can "
           "optionally be k or m";
  return "";
}

std::string isMethod(std::string const &value, size_t index) {
  (void)index;
  std::string methods[] = HTTP_METHODS;
  for (size_t i = 0; i < sizeof(methods) / sizeof(std::string); i++)
    if (value == methods[i]) return "";
  return "Invalid method";
}

std::string isLogLevel(std::string const &value, size_t index) {
  (void)index;
  if (value == "debug" || value == "info" || value == "warning" ||
      value == "error" || value == "verbose")
    return "";
  return "Invalid log level";
}

std::string isAbsolutePath(std::string const &value, size_t index) {
  (void)index;
  if (value.length() == 0) return "Path cannot be empty";
  if (value[0] != '/') return "Path must start with a '/'";
  return "";
}

std::string isExtension(std::string const &value, size_t index) {
  (void)index;
  if (value.length() == 0) return "Extension cannot be empty";
  for (size_t i = 0; i < value.length(); i++)
    if (value[i] == '.')
      return "'.' not allowed in extension, only use the part after '.' as "
             "extension";
  return "";
}

std::string isBoolean(std::string const &value, size_t index) {
  (void)index;
  if (value == "on" || value == "off") return "";
  return "Invalid boolean value";
}

std::string isListen(std::string const &value, size_t index) {
  if (value.empty()) return "Invalid listen format (empty)";
  if (value.size() <= 5) return isNumeric(value, index);

  bool ipv6 = value[0] == '[';
  std::string address, port;
  if (ipv6) {
    size_t pos = value.find("]:");
    if (pos == std::string::npos)
      return "Invalid listen ipv6 format (missing port)";
    address = value.substr(1, pos - 1);
    port = value.substr(pos + 2);
  } else {
    size_t pos = value.find(":");
    if (pos == std::string::npos)
      return "Invalid listen ipv4 format (missing port)";
    address = value.substr(0, pos);
    port = value.substr(pos + 1);
  }
  if (isNumeric(port, index) != "") return "Invalid listen port";
  if (ipv6 == true && address.empty()) return "Invalid listen ipv6 address";
  if (ipv6 == false && address.empty()) return "Invalid listen ipv4 address";
  return "";
}

std::string isCgi(std::string const &value, size_t index) {
  if (index == 0) return isExtension(value, index);
  return isAbsolutePath(value, index);
}

Config::Config() {}

Config::Config(std::string path, std::string includePath) {
  _path = path.empty() ? CONFIG_PATH : path;
  _includePath =
      includePath.empty() ? File(_path).getDir() : File(includePath).getDir();

  File f(path);
  if (f.exists() == false)
    throw std::runtime_error("File '" + path + "' does not exist");
  std::ifstream fstream(_path.c_str());
  if (fstream.is_open() == false)
    throw std::runtime_error("Failed to open file '" + path + "'");
  std::stringstream stream;
  stream << fstream.rdbuf();
  if (stream.fail())
    throw std::runtime_error("Failed to read file '" + path + "'");
  _config = stream.str();
}

Config::Config(const Config &rhs) { *this = rhs; }

Config &Config::operator=(const Config &rhs) {
  if (this == &rhs) return *this;
  _path = rhs._path;
  _config = rhs._config;
  _includePath = rhs._includePath;
  return *this;
}

Config::~Config() {}

std::string Config::getConfig() const { return _config; }

void Config::removeComments() {
  accessLog_g.write(_path + " Removing comments", DEBUG);
  size_t i = _config.find_first_of("#");
  while (i != std::string::npos) {
    size_t j = _config.find_first_of("\n", i);
    if (j == std::string::npos) j = _config.length();
    _config.erase(i, j - i);
    i = _config.find_first_of("#");
  }
}

Context &Config::parseContext(Context &context, std::string data, size_t line,
                              bool validateResult) {
  accessLog_g.write(_path + ":" + toString(line) + " Context: '" + context.getName() + "' -> Parsing", DEBUG);
  size_t startLine = line;
  while (true) {
    // Trim leading whitespace
    line += linesUntilPos(data, data.find_first_not_of(" \f\n\r\t\v"));
    trimStart(data);
    if (data.length() == 0) break;

    // Find end of token
    size_t nextEnd = data.find_first_of(" \t\n");
    if (nextEnd == std::string::npos || data[nextEnd] == '\n')
      throwExeption(line, "Unexpected token '\\n' found");
    line += linesUntilPos(data, nextEnd);

    // Process token
    std::string token = trim(cut(data, 0, nextEnd));
    if (token == "include" || isValidDirective(context, token)) {
      // Find end of token value
      nextEnd = data.find_first_of(";\n");
      if (nextEnd == std::string::npos || data[nextEnd] != ';')
        throwExeption(line, "Expected token ';' not found");

      // Process token value
      if (token == "include") try {
          processInclude(context, trim(cut(data, 0, nextEnd)));
        } catch (const std::exception &e) {
          throwExeption(line, e.what());
        }
      else {
        checkError(line, validToAdd(context, token));
        std::vector<std::string> args = split<std::vector<std::string> >(
            cut(data, 0, nextEnd), " \f\n\r\t\v");
        checkError(line, validArguments(context, token, args));
        context.addDirective(token, args);
      }
      data.erase(0, 1);
    } else if (isValidContext(context, token)) {
      processContext(context, data, token, line);
    } else
      throwExeption(line, "Invalid token '" + token + "'");
  }
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
  std::vector<std::string> args = split<std::vector<std::string> >(
      trim(cut(data, 0, argsEnd)), " \f\n\r\t\v");
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
  // Prepare wildcard path
  std::string includePath;
  if (startsWith(path, "/"))
    includePath = path;
  else
    includePath = _includePath + path;

  // Get file list
  std::set<std::string> files = processWildcardPath(includePath);
  std::set<std::string>::iterator itr = files.begin();
  for (; itr != files.end(); itr++) {
    accessLog_g.write( _path +
        " Context: '" + context.getName() + "' -> Include '" + *itr + "'",
        DEBUG);
    // Recursively parse included config files
    Config config(*itr, _includePath);
    config.removeComments();
    config.parseContext(context, config.getConfig(), 1, false);
  }
}

bool Config::isValidContext(Context &context, std::string token) const {
  for (size_t i = 0; i < sizeof(tokens_g) / sizeof(token_t); i++)
    if (tokens_g[i].name == token && tokens_g[i].parent == context.getName() &&
        tokens_g[i].isContext)
      return true;
  return false;
}

bool Config::isValidDirective(Context &context, std::string token) const {
  for (size_t i = 0; i < sizeof(tokens_g) / sizeof(token_t); i++)
    if (tokens_g[i].name == token && tokens_g[i].parent == context.getName() &&
        !tokens_g[i].isContext)
      return true;
  return false;
}

std::string Config::validToAdd(Context &context, std::string token) {
  for (size_t i = 0; i < sizeof(tokens_g) / sizeof(token_t); i++)
    if (tokens_g[i].name == token && tokens_g[i].parent == context.getName())
      return context.getTokenOccurence(token) < tokens_g[i].maxOccurence
                 ? ""
                 : "Token '" + token + "'has too many occurences";
  return "Token '" + token + "' not found";
}

std::string Config::validArguments(Context &context, std::string token,
                                   std::vector<std::string> args) {
  for (size_t i = 0; i < sizeof(tokens_g) / sizeof(token_t); i++) {
    if (tokens_g[i].name == token && tokens_g[i].parent == context.getName()) {
      if (tokens_g[i].func != NULL) {
        for (size_t j = 0; j < args.size(); j++) {
          std::string error = tokens_g[i].func(args[j], j);
          if (error != "") return "Argument '" + args[j] + "': " + error;
        }
      }
      if (tokens_g[i].minArgs <= args.size() &&
          tokens_g[i].maxArgs >= args.size())
        return "";
      return "'" + token + "' requires between " +
             toString(tokens_g[i].minArgs) + " and " +
             toString(tokens_g[i].maxArgs) + " arguments";
    }
  }
  return "Token '" + token + "' not found";
}

std::string Config::validate(Context &context, bool recursive) {
  accessLog_g.write(_path + " Context: '" + context.getName() + "' -> Validating",
                    DEBUG);
  for (size_t i = 0; i < sizeof(tokens_g) / sizeof(token_t); i++) {
    if (tokens_g[i].parent == context.getName() &&
        context.getTokenOccurence(tokens_g[i].name) <
            tokens_g[i].minOccurence) {
      if (tokens_g[i].isContext)
        return "Context missing required context '" + tokens_g[i].name + "'";
      return "Context missing required directive '" + tokens_g[i].name + "'";
    }
  }
  if (recursive) {
    std::map<std::string, std::vector<Context> >::iterator it;
    for (it = context.getContexts().begin(); it != context.getContexts().end();
         it++)
      for (size_t i = 0; i < it->second.size(); i++) validate(context, true);
  }
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
  throw std::runtime_error(_path + ":" + toString(line) + ": " + msg);
}
