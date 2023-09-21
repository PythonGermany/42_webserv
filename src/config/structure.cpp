#include "structure.hpp"

std::string isNumeric(std::string const &value) {
  for (size_t i = 0; i < value.length(); i++)
    if (std::isdigit(value[i]) == false)
      return "All characters must be numeric";
  return "";
}

std::string isMethod(std::string const &value) {
  if (value == "GET" || value == "HEAD" || value == "POST" || value == "DELETE")
    return "";
  return "Invalid method";
}

std::string isLogLevel(std::string const &value) {
  if (value == "DEBUG" || value == "INFO" || value == "WARNING") return "";
  return "Invalid log level";
}

std::string isAbsolutePath(std::string const &value) {
  if (value.length() == 0) return "Path cannot be empty";
  if (value[0] != '/') return "Path must start with a '/'";
  return "";
}

std::string isExtension(std::string const &value) {
  if (value.length() == 0) return "Extension cannot be empty";
  for (size_t i = 0; i < value.length(); i++)
    if (std::isalpha(value[i]) == false)
      return "Extension must contain only letters";
  return "";
}

std::string isBoolean(std::string const &value) {
  if (value == "on" || value == "off") return "";
  return "Invalid boolean value";
}

std::string isListen(std::string const &value) {
  if (value.empty()) return "Invalid listen format (empty)";

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
  if (isNumeric(port) != "") return "Invalid listen port";
  if (ipv6 && address.empty()) return "Invalid listen ipv6 address";
  if (!ipv6 && address.empty()) return "Invalid listen ipv4 address";
  return "";
}