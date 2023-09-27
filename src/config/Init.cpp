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
  Log::write(toString(size) + " virtual hosts created", INFO);
  Log::write("------- Virtual Hosts loaded --------", INFO, BRIGHT_GREEN);
}

void Init::initMimeTypes(Context& context) {
  Log::write("-------- Loading mime types ---------", INFO, BRIGHT_GREEN);
  std::map<std::string, std::string> types;
  std::vector<std::vector<std::string> >& mimes =
      context.getContext("http")[0].getContext("types")[0].getDirective("type");
  for (size_t i = 0; i < mimes.size(); i++) {
    std::string mime = mimes[i][0];
    for (size_t j = 1; j < mimes[i].size(); j++) {
      if (types.find(mimes[i][j]) != types.end())
        Log::write(
            "WARNING: Duplicate mime type extension '" + mimes[i][j] + "'",
            WARNING, YELLOW);
      types[mimes[i][j]] = mime;
    }
  }
  context.getContext("http")[0].removeContext("types");
  VirtualHost::setMimeTypes(types);
  Log::write("Mime types loaded for " + toString(types.size()) + " extensions",
             INFO);
  Log::write("-- Mime types successfully loaded ---", INFO, BRIGHT_GREEN);
}

void Init::initLogDefaults(Context& context) {
  Context& http = context.getContext("http")[0];
  // Init log to stdout
  if (http.exists("log_to_stdout"))
    Log::setLogToStdout(http.getDirective("log_to_stdout")[0][0] == "on");
  // Init log level
  if (http.exists("log_level")) {
    std::string level = http.getDirective("log_level")[0][0];
    if (level == "debug")
      Log::setLevel(DEBUG);
    else if (level == "info")
      Log::setLevel(INFO);
    else if (level == "warning")
      Log::setLevel(WARNING);
    else if (level == "error")
      Log::setLevel(ERROR);
  }
  // Init log files
  if (http.exists("access_log"))
    Log::setLogFile(http.getDirective("access_log")[0][0]);
  if (http.exists("error_log"))
    Log::setErrorLogFile(http.getDirective("error_log")[0][0]);
}

void Init::initPoll() {
  Log::write("--------- Creating sockets ----------", INFO, BRIGHT_GREEN);
  size_t sockets = 0;
  std::vector<VirtualHost>& virtualHosts = VirtualHost::getVirtualHosts();
  std::set<Address> allAddresses;

  for (size_t i = 0; i < virtualHosts.size(); i++) {
    std::set<Address> const& toadd = virtualHosts[i].getResolvedAddress();
    allAddresses.insert(toadd.begin(), toadd.end());
  }
  for (std::set<Address>::const_iterator it = allAddresses.begin();
       it != allAddresses.end(); ++it) {
    std::set<Address>::const_iterator addr_any =
        allAddresses.find(Address(it->family(), it->port()));
    if (addr_any == allAddresses.end() || addr_any == it) {
      Poll::add(new ListenSocket(*it));
      ++sockets;
    }
  }
  {
    Log::write("Number of sockets: " + toString(sockets), INFO);
    Log::write("---------- Sockets created ----------", INFO, BRIGHT_GREEN);
  }
}