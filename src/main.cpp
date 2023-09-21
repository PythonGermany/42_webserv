#include "Config.hpp"
#include "Init.hpp"
#include "ListenSocket.hpp"
#include "Poll.hpp"
#include "webserv.hpp"

Context loadConfig(std::string path) {
  Log::write("-------- Loading config file --------", INFO, BRIGHT_GREEN);
  Context context("_", NULL);
  Config config(path);
  config.removeComments();
  context = config.parseContext(context, config.getConfig());
  if (LOG_LEVEL >= DEBUG) context.print();
  Log::write("-- Config file successfully loaded --", INFO, BRIGHT_GREEN);
  return context;
}

int main(int argc, char** argv) {
  try {
    Context context = loadConfig(argc > 1 ? argv[1] : CONFIG_PATH);
    Init::init(context);
    Poll::poll();
  } catch (const std::exception& e) {
    Log::writeError(e.what(), BRIGHT_YELLOW);
  }
  Log::close();
  Log::write("Number of open files: " + toString(File::getFilesOpen()), INFO);
  return 0;
}
