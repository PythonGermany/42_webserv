#include "Config.hpp"
#include "Log.hpp"
#include "webserv.hpp"

int loadConfig(Context& context, std::string path) {
  Log::write("-------- Loading config file --------", INFO, BRIGHT_GREEN);
  try {
    Config config(path);
    config.removeComments();
    context = config.parseContext(config.getConfig(), "_", "");
  } catch (std::exception& e) {
    Log::writeError(e.what(), BRIGHT_YELLOW);
    return 1;
  }
  if (LOG_LEVEL >= DEBUG) context.print();
  Log::write("-- Config file successfully loaded --", INFO, BRIGHT_GREEN);
  return 0;
}

int main(int argc, char** argv) {
  Context context;

  Log::init();
  Log::write("--------- Starting webserv ----------", INFO, BRIGHT_GREEN);
  if (loadConfig(context, argc > 1 ? argv[1] : CONFIG_PATH) == 1)
    Log::writeError("Error while loading config file", BRIGHT_RED);
  Log::write("Number of open files: " + toString(File::getFilesOpen()), INFO);
  Log::write("---------- Stopped webserv ----------", INFO, BRIGHT_GREEN);
  Log::close();
}