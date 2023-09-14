#include "Config.hpp"
#include "Init.hpp"
#include "webserv.hpp"

int loadConfig(Context& context, std::string path) {
  Log::write("-------- Loading config file --------", INFO, BRIGHT_GREEN);
  try {
    Config config(path);
    config.removeComments();
    context = config.parseContext(context, config.getConfig());
  } catch (std::exception& e) {
    Log::writeError(e.what(), BRIGHT_YELLOW);
    return 1;
  }
  if (LOG_LEVEL >= DEBUG) context.print();
  Log::write("-- Config file successfully loaded --", INFO, BRIGHT_GREEN);
  return 0;
}

int main(int argc, char** argv) {
  Context context("_", "");
  std::map<std::string, std::string> mimeTypes;
  std::vector<Server> servers;

  if (loadConfig(context, argc > 1 ? argv[1] : CONFIG_PATH) == 1)
    Log::writeError("Error while loading config file", BRIGHT_RED);
  else {
    Init::initLogDefaults(context);
    mimeTypes = Init::initMimeTypes(context);
    servers = Init::initServers(context, mimeTypes);
  }
  Log::close();
  Log::write("Number of open files: " + toString(File::getFilesOpen()), INFO);
  return 0;
}