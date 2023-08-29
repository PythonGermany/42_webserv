#include "Config.hpp"

Config::Config() {}

Config::Config(std::string path) { setFile(path); }

Config::Config(const Config &rhs) { *this = rhs; }

Config &Config::operator=(const Config &rhs) {
  if (this == &rhs) return *this;
  _file = rhs._file;
  return *this;
}

Config::~Config() {}

std::string Config::getFile() { return _file.Read(); }

void Config::setFile(std::string path) {
  _file = File(path);
  if (!_file.exists()) throwExeption("setFile", "File does not exist");
  if (!_file.file()) throwExeption("setFile", "Path is not a file");
  if (!_file.readable()) throwExeption("setFile", "File is not readable");
  _config = _file.Read();
}

std::vector<Server> Config::parseConfig() {
  std::vector<Server> servers;
  std::string data = trim(_config);

  while (data.length() > 0) {
    std::string token = trim(cut(data, 0, findToken(data, " ")));
    if (token != "server")
      throwExeption("parseConfig", "Unknown token '" + token + "'");
    std::string contextData = trim(cut(data, 0, findContextEnd(data)));
    servers.push_back(Server(parseContext(contextData, token, "_")));
    data = trim(data);
  }
  return servers;
}

void printVector(std::vector<std::string> vec) {
  for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();
       it++)
    std::cout << "'" << *it << "' ";
  std::cout << std::endl;
}

Context Config::parseContext(std::string data, std::string name,
                             std::string parent) {
  Context context(name, parent);
  if (startsWith(data, "{") == false)
    throwExeption("parseContext", "Expected token '{' not found");
  data = trim(cut(data, 1, findContextEnd(data) - 1));
  while (data.length() > 0) {
    std::string token = trim(cut(data, 0, findToken(data, " ")));
    if (context.validTokenName(token) == false)
      throwExeption("parseContext", "Unknown token '" + token + "'");
    if (context.isContext(token)) {
      std::string contextData = trim(cut(data, 0, findContextEnd(data)));
      Context childContext = parseContext(contextData, token, name);
      context.addContext(childContext);
    } else if (context.isDirective(token)) {
      std::vector<std::string> values =
          split(cut(data, 0, findToken(data, ";")), " ");
      if (context.validDirective(token, values) == false)
        throwExeption("parseContext",
                      "Invalid values for token '" + token + "'");
      context.addDirectives(token, values);
      data = data.substr(1);
    }
    data = trim(data);
  }
  return context;
}

void Config::validateConfig(
    std::vector<Server>
        &servers) {  // TODO: complete, improve and test validation
  (void)servers;
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
    if (depth == 0) return i + 1;
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