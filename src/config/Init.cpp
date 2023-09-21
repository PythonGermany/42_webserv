#include "Init.hpp"

Init::Init() {}

Init::~Init() {}

void Init::init(Context& context) {
  initLogDefaults(context);
  initMimeTypes(context);
  initVirtualHosts(context);
  initPoll();
}

void Init::initVirtualHosts(Context& context) {
  Log::write("------- Loading Virtual Hosts -------", INFO, BRIGHT_GREEN);
  std::vector<Context>& serverContexts =
      context.getContext("http")[0].getContext("server");
  for (size_t i = 0; i < serverContexts.size(); i++) {
    serverContexts[i].setParent(NULL);
    VirtualHost::add(VirtualHost(serverContexts[i]));
  }
  size_t size = VirtualHost::getVirtualHosts().size();
  {
    Log::write("Number of Virtual Hosts: " + toString(size), INFO);
    Log::write("------- Virtual Hosts loaded --------", INFO, BRIGHT_GREEN);
  }
}

void Init::initMimeTypes(Context& context) {
  Log::write("-------- Loading mime types ---------", INFO, BRIGHT_GREEN);
  std::map<std::string, std::string> types;
  std::vector<Context>& mimes =
      context.getContext("http")[0].getContext("types")[0].getContext("type");
  for (size_t i = 0; i < mimes.size(); i++) {
    std::string mime = mimes[i].getArgs()[0];
    std::vector<std::string> exts = mimes[i].getDirective("extension");
    for (size_t j = 0; j < exts.size(); j++) {
      if (types.find(exts[j]) != types.end())
        Log::write("WARNING: Duplicate mime type extension '" + exts[j] + "'",
                   WARNING, YELLOW);
      types[exts[j]] = mime;
    }
  }
  context.getContext("http")[0].removeContext("types");
  VirtualHost::setMimeTypes(types);
  {
    Log::write("Number of mime extensions: " + toString(types.size()), INFO);
    Log::write("-- Mime types successfully loaded ---", INFO, BRIGHT_GREEN);
  }
}

void Init::initLogDefaults(Context& context) {
  Context& http = context.getContext("http")[0];
  // Init log level
  if (http.exists("log_level")) {
    std::string level = http.getDirective("log_level")[0];
    if (level == "DEBUG")
      Log::setLevel(DEBUG);
    else if (level == "INFO")
      Log::setLevel(INFO);
    else if (level == "WARNING")
      Log::setLevel(WARNING);
  }

  // Init log files
  if (http.exists("access_log"))
    Log::setLogFile(http.getDirective("access_log")[0]);
  if (http.exists("error_log"))
    Log::setErrorLogFile(http.getDirective("error_log")[0]);
  Log::init();
  {
    Log::write("--------- Log configuration ---------", INFO, BRIGHT_GREEN);
    Log::write("Log level: " + toString(Log::getLevel()), INFO);
    Log::write("Access log file: " + Log::getLogFile().getPath(), INFO);
    Log::write("Error log file: " + Log::getErrorLogFile().getPath(), INFO);
  }
}

void Init::initPoll() {  // TODO: Implement correct socket creation
  Log::write("--------- Creating sockets ----------", INFO, BRIGHT_GREEN);
  size_t sockets = 0;
  std::vector<VirtualHost>& virtualHosts = VirtualHost::getVirtualHosts();
  for (size_t i = 0; i < virtualHosts.size(); i++) {
    Poll::add(new ListenSocket(virtualHosts[i].getAddress()));
    sockets++;
  }
  {
    Log::write("Number of sockets: " + toString(sockets), INFO);
    Log::write("---------- Sockets created ----------", INFO, BRIGHT_GREEN);
  }
}