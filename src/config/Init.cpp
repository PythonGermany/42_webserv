#include "Init.hpp"

Init::Init() {}

Init::~Init() {}

void Init::init(Poll& poll, Context& context) {
  Context& http = context.getContext("http")[0];

  initLogDefaults(http);
  initMimeTypes(http);
  initVirtualHosts(http);
  initPoll(poll);
}

void Init::initLogDefaults(Context& context) {
  // Init log to terminal
  if (context.exists("log_to_terminal"))
    Log::setLogToTerminal(context.getDirective("log_to_terminal")[0][0] ==
                          "on");
  // Init log level
  if (context.exists("log_level")) {
    std::string level = context.getDirective("log_level")[0][0];

    if (level == "debug")
      Log::setLevel(DEBUG);
    else if (level == "info")
      Log::setLevel(INFO);
    else if (level == "warning")
      Log::setLevel(WARNING);
    else if (level == "error")
      Log::setLevel(ERROR);
    else if (level == "verbose")
      Log::setLevel(VERBOSE);
  }
  // Init log files
  if (context.exists("access_log"))
    accessLog_g.setFile(context.getDirective("access_log")[0][0]);
  if (context.exists("error_log"))
    errorLog_g.setFile(context.getDirective("error_log")[0][0]);
  accessLog_g.setInitialized(true);
  errorLog_g.setInitialized(true);
}

void Init::initMimeTypes(Context& context) {
  accessLog_g.write("-------- Loading mime types ---------", INFO,
                    BRIGHT_GREEN);
  std::map<std::string, std::string> types;
  std::vector<std::vector<std::string> >& mimes =
      context.getContext("types")[0].getDirective("type");
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
  context.removeContext("types");
  VirtualHost::setMimeTypes(types);
  accessLog_g.write(
      "Mime types loaded for " + toString(types.size()) + " extensions", INFO);
  accessLog_g.write("-- Mime types successfully loaded ---", INFO,
                    BRIGHT_GREEN);
}

void Init::initVirtualHosts(Context& context) {
  accessLog_g.write("------- Loading Virtual Hosts -------", INFO,
                    BRIGHT_GREEN);
  std::vector<Context>& serverContexts = context.getContext("server");

  for (size_t i = 0; i < serverContexts.size(); i++) {
    serverContexts[i].setParent(NULL);
    VirtualHost::add(VirtualHost(serverContexts[i]));
  }
  size_t size = VirtualHost::getVirtualHosts().size();
  accessLog_g.write(toString(size) + " virtual hosts loaded", INFO);
  accessLog_g.write("------- Virtual Hosts loaded --------", INFO,
                    BRIGHT_GREEN);
}

void Init::initPoll(Poll& poll) {
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
      ListenSocket* s = NULL;
      try {
        s = new ListenSocket(*it);
        if (poll.add(s)) throw std::exception();
      } catch (...) {
        delete s;
        throw std::runtime_error("Failed to initialize listen socket");
      }
      ++sockets;
    }
  }
  accessLog_g.write("Number of sockets: " + toString(sockets), INFO);
  accessLog_g.write("---------- Sockets created ----------", INFO,
                    BRIGHT_GREEN);
}
