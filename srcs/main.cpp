#include "Config.hpp"
#include "Log.hpp"
#include "webserv.hpp"

void initDefaults(Context& context) {
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
}

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

  if (loadConfig(context, argc > 1 ? argv[1] : CONFIG_PATH) == 1)
    Log::writeError("Error while loading config file", BRIGHT_RED);
  else {
    initDefaults(context);
  }
  Log::close();
  Log::write("Number of open files: " + toString(File::getFilesOpen()), INFO);
  return 0;
}