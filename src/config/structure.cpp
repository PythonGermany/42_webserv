#include "structure.hpp"

#include "global.hpp"

arg_state_t setInfo(const std::list<std::string> &values) {
  static bool set = false;
  if (values.size() > 0) return ARG_INVALID;
  if (set) return FLAG_DUPLICATE;
  printInfo(SET);
  set = true;
  return SUCCESS;
}

arg_state_t setHelp(const std::list<std::string> &values) {
  static bool set = false;
  if (values.size() > 0) return ARG_INVALID;
  if (set) return FLAG_DUPLICATE;
  printHelp(SET);
  set = true;
  return SUCCESS;
}

arg_state_t setLogToStdout(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  std::string value = values.front();
  Log::setLogToStdout(value == "on");
  set = true;
  if (value == "on" || value == "off") return SUCCESS;
  return ARG_INVALID;
}

arg_state_t setLogLevel(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  std::string value = values.front();
  Log::setLevel((log_level_t)(value[0] - '0'));
  set = true;
  if (value.size() == 1 && value >= "0" && value <= "3") return SUCCESS;
  return ARG_INVALID;
}

arg_state_t setAccessLog(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  accessLog_g.setFile(values.front());
  accessLog_g.setInitialized(true);
  set = true;
  return SUCCESS;
}

arg_state_t setErrorLog(const std::list<std::string> &values) {
  static bool set = false;
  if (set) return FLAG_DUPLICATE;
  errorLog_g.setFile(values.front());
  errorLog_g.setInitialized(true);
  set = true;
  return SUCCESS;
}

std::string isMimeType(std::string const &value, size_t index) {
  if (value.length() == 0) return "Mime type cannot be empty";
  if (index == 0) {
    if (value.find('/') == std::string::npos)
      return "Mime type must contain a '/'";
  } else
    return isExtension(value, index);
  return "";
}

std::string isErrorPage(std::string const &value,
                        size_t index) {  // TODO find better way to do this
  if (index == 0) {
    if (value == "100" || value == "101" || value == "300" || value == "400" ||
        value == "401" || value == "402" || value == "403" || value == "404" ||
        value == "405" || value == "406" || value == "407" || value == "408" ||
        value == "409" || value == "410" || value == "411" || value == "412" ||
        value == "413" || value == "414" || value == "415" || value == "416" ||
        value == "417" || value == "500" || value == "501" || value == "502" ||
        value == "503" || value == "504" || value == "505")
      return "";
    return "Invalid error code";
  } else
    return isAbsolutePath(value, index);
}

std::string isNumeric(std::string const &value, size_t index) {
  (void)index;
  for (size_t i = 0; i < value.length(); i++)
    if (std::isdigit(value[i]) == false)
      return "All characters must be numeric";
  return "";
}

std::string isMethod(std::string const &value, size_t index) {
  (void)index;
  if (value == "GET" || value == "HEAD" || value == "OPTIONS" ||
      value == "PUT" || value == "POST" || value == "DELETE")
    return "";
  return "Invalid method";
}

std::string isLogLevel(std::string const &value, size_t index) {
  (void)index;
  if (value == "debug" || value == "info" || value == "warning" ||
      value == "error")
    return "";
  return "Invalid log level";
}

std::string isAbsolutePath(std::string const &value, size_t index) {
  (void)index;
  if (value.length() == 0) return "Path cannot be empty";
  if (value[0] != '/') return "Path must start with a '/'";
  return "";
}

std::string isExtension(std::string const &value, size_t index) {
  (void)index;
  if (value.length() == 0) return "Extension cannot be empty";
  for (size_t i = 0; i < value.length(); i++)
    if (value[i] == '.')
      return "'.' not allowed in extension, only use the part after '.' as "
             "extension";
  return "";
}

std::string isBoolean(std::string const &value, size_t index) {
  (void)index;
  if (value == "on" || value == "off") return "";
  return "Invalid boolean value";
}

std::string isListen(std::string const &value, size_t index) {
  if (value.empty()) return "Invalid listen format (empty)";
  if (value.size() <= 5) return "";  // TODO: Ask what this is for

  bool ipv6 = value[0] == '[';
  std::string address, port;
  if (ipv6) {
    size_t pos = value.find("]:");
    if (pos == std::string::npos)
      return "Invalid listen ipv6 format (missing port)";
    address = value.substr(1, pos - 1);
    port = value.substr(pos + 2);
  } else {
    size_t pos = value.find(":");
    if (pos == std::string::npos)
      return "Invalid listen ipv4 format (missing port)";
    address = value.substr(0, pos);
    port = value.substr(pos + 1);
  }
  if (isNumeric(port, index) != "") return "Invalid listen port";
  if (ipv6 && address.empty()) return "Invalid listen ipv6 address";
  if (!ipv6 && address.empty()) return "Invalid listen ipv4 address";
  return "";
}