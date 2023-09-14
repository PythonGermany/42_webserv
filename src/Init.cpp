#include "Init.hpp"

Init::Init() {}

Init::~Init() {}

std::vector<VirtualHost> Init::initVirtualHosts(Context& context) {
  Log::write("-------- Loading servers -----------", INFO, BRIGHT_GREEN);
  std::vector<VirtualHost> servers;
  std::vector<Context>& serverContexts =
      context.getContext("http")[0].getContext("server");
  for (size_t i = 0; i < serverContexts.size(); i++)
    servers.push_back(VirtualHost(serverContexts[i]));
  Log::write("Number of servers: " + toString(servers.size()), INFO);
  Log::write("-- Servers successfully loaded ------", INFO, BRIGHT_GREEN);
  return servers;
}

void Init::initMimeTypes(Context& context) {
  Log::write("-------- Loading mime types ---------", INFO, BRIGHT_GREEN);
  std::map<std::string, std::string> types;
  std::vector<Context>& mimes =
      context.getContext("http")[0].getContext("types")[0].getContext("type");
  for (size_t i = 0; i < mimes.size(); i++) {
    std::string mime = mimes[i].getDirective("mime")[0];
    std::vector<std::string> exts = mimes[i].getDirective("extension");
    for (size_t j = 0; j < exts.size(); j++) {
      if (types.find(exts[j]) != types.end())
        Log::write("WARNING: Duplicate mime type extension '" + exts[j] + "'",
                   WARNING, YELLOW);
      types[exts[j]] = mime;
    }
  }
  context.getContext("http")[0].removeContext("types");
  Log::write("Number of mime extensions: " + toString(types.size()), INFO);
  VirtualHost::setMimeTypes(types);
  Log::write("-- Mime types successfully loaded ---", INFO, BRIGHT_GREEN);
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