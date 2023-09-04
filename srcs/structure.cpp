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