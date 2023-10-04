#include "Init.hpp"

Init::Init() {}

Init::~Init() {}

void Init::init(Context context) {
  initLogDefaults(context);
  initMimeTypes(context);
  initVirtualHosts(context);
  initPoll();
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
    accessLog_g.setFile(http.getDirective("access_log")[0][0]);
  if (http.exists("error_log"))
    errorLog_g.setFile(http.getDirective("error_log")[0][0]);
  accessLog_g.setInitialized(true);
  errorLog_g.setInitialized(true);
}

void Init::initMimeTypes(Context& context) {
  accessLog_g.write("-------- Loading mime types ---------", INFO,
                    BRIGHT_GREEN);
  std::map<std::string, std::string> types;
  std::vector<std::vector<std::string> >& mimes =
      context.getContext("http")[0].getContext("types")[0].getDirective("type");
  for (size_t i = 0; i < mimes.size(); i++) {
    std::string mime = mimes[i][0];
    for (size_t j = 1; j < mimes[i].size(); j++) {
      if (types.find(mimes[i][j]) != types.end())
        accessLog_g.write(
            "WARNING: Duplicate mime type extension '" + mimes[i][j] + "'",
            WARNING, YELLOW);
      types[mimes[i][j]] = mime;
    }
  }
  context.getContext("http")[0].removeContext("types");
  VirtualHost::setMimeTypes(types);
  accessLog_g.write(
      "Mime types loaded for " + toString(types.size()) + " extensions", INFO);
  accessLog_g.write("-- Mime types successfully loaded ---", INFO,
                    BRIGHT_GREEN);
}

void Init::initVirtualHosts(Context& context) {
  accessLog_g.write("------- Loading Virtual Hosts -------", INFO,
                    BRIGHT_GREEN);
  std::vector<Context>& serverContexts =
      context.getContext("http")[0].getContext("server");

  for (size_t i = 0; i < serverContexts.size(); i++) {
    serverContexts[i].setParent(NULL);
    VirtualHost::add(VirtualHost(serverContexts[i]));
  }
  size_t size = VirtualHost::getVirtualHosts().size();
  accessLog_g.write(toString(size) + " virtual hosts loaded", INFO);
  accessLog_g.write("------- Virtual Hosts loaded --------", INFO,
                    BRIGHT_GREEN);
}

void Init::initPoll() {
  accessLog_g.write("--------- Creating sockets ----------", INFO,
                    BRIGHT_GREEN);
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
      ListenSocket::create(*it);
      ++sockets;
    }
  }
  accessLog_g.write("Number of sockets: " + toString(sockets), INFO);
  accessLog_g.write("---------- Sockets created ----------", INFO,
                    BRIGHT_GREEN);
}