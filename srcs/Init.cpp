#include "Init.hpp"

Init::Init() {}

Init::~Init() {}

std::vector<Server> Init::initServers(
    Context& context, std::map<std::string, std::string> mimeTypes) {
  Log::write("-------- Loading servers -----------", INFO, BRIGHT_GREEN);
  Server::setMimeTypes(mimeTypes);
  std::vector<Server> servers;
  std::vector<Context> serverContexts =
      context.getContext("http")[0].getContext("server");
  for (std::vector<Context>::iterator it = serverContexts.begin();
       it != serverContexts.end(); it++) {
    Server server(*it);
    servers.push_back(server);
  }
  Log::write("Number of servers: " + toString(servers.size()), INFO);
  Log::write("-- Servers successfully loaded ------", INFO, BRIGHT_GREEN);
  return servers;
}

std::map<std::string, std::string> Init::initMimeTypes(Context& context) {
  Log::write("-------- Loading mime types ---------", INFO, BRIGHT_GREEN);
  std::map<std::string, std::string> types;
  std::vector<Context>& mimes =
      context.getContext("http")[0].getContext("types")[0].getContext("type");
  for (std::vector<Context>::iterator it = mimes.begin(); it != mimes.end();
       it++) {
    std::string mime = it->getDirective("mime")[0];
    std::vector<std::string> exts = it->getDirective("extension");
    for (std::vector<std::string>::iterator it2 = exts.begin();
         it2 != exts.end(); it2++) {
      if (types.find(*it2) != types.end())
        Log::write("WARNING: Duplicate mime type extension '" + *it2 + "'",
                   WARNING, YELLOW);
      types[*it2] = mime;
    }
  }
  Log::write("Number of mime extensions: " + toString(types.size()), INFO);
  Log::write("-- Mime types successfully loaded ---", INFO, BRIGHT_GREEN);
  return types;
}

void Init::initLogDefaults(Context& context) {
  Context& http = context.getContext("http")[0];
  if (http.exists("log_level")) {
    std::string level = http.getDirective("log_level")[0];
    if (level == "DEBUG")
      Log::setLevel(DEBUG);
    else if (level == "INFO")
      Log::setLevel(INFO);
    else if (level == "WARNING")
      Log::setLevel(WARNING);
  }
  if (http.exists("access_log"))
    Log::setLogFile(http.getDirective("access_log")[0]);
  if (http.exists("error_log"))
    Log::setErrorLogFile(http.getDirective("error_log")[0]);
  Log::init();
  Log::write("--------- Log configuration ---------", INFO, BRIGHT_GREEN);
  Log::write("Log level: " + toString(Log::getLevel()), INFO);
  Log::write("Access log file: " + Log::getLogFile().getPath(), INFO);
  Log::write("Error log file: " + Log::getErrorLogFile().getPath(), INFO);
}