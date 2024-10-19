#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <set>
#include <string>
#include <vector>

#include "Context.hpp"
#include "File.hpp"
#include "Http.hpp"
#include "global.hpp"
#include "utils.hpp"

// ----------------------- CONFIG STRUCTURE ------------------------
typedef struct token_s {
  std::string name;
  std::string parent;
  bool isContext;
  size_t minOccurence;
  size_t maxOccurence;
  size_t minArgs;
  size_t maxArgs;
  std::string (*func)(std::string const &value, size_t index);
} token_t;

// Validation functions are return a non-empty string if value is invalid
std::string isMimeType(std::string const &value, size_t index);
std::string isErrorPage(std::string const &value, size_t index);
std::string isNumeric(std::string const &value, size_t index);
std::string isMemorySize(std::string const &value, size_t index);
std::string isMethod(std::string const &value, size_t index);
std::string isLogLevel(std::string const &value, size_t index);
std::string isAbsolutePath(std::string const &value, size_t index);
std::string isExtension(std::string const &value, size_t index);
std::string isBoolean(std::string const &value, size_t index);
std::string isListen(std::string const &value, size_t index);
std::string isCgi(std::string const &value, size_t index);

// WEBSERV_CONFIG ---------- CONFIG VALUES -------------------------
#define CONFIG_PATH "/etc/webserv/webserv.conf"

/**
 * Allowed token input structure in the format: {name, parent, isContext,
 * minOccurence, maxOccurence, minArgs, maxArgs, validationFunction}
 */
const token_t tokens_g[] = {
    // Wildcard context
    {"include", "*", false, 0, static_cast<size_t>(-1), 1, 1, NULL},

    // Root context
    {"http", "_", true, 1, 1, 0, 0, NULL},

    // Http context
    {"log_to_terminal", "http", false, 0, 1, 1, 1, isBoolean},
    {"log_level", "http", false, 0, 1, 1, 1, isLogLevel},
    {"access_log", "http", false, 0, 1, 1, 1, NULL},
    {"error_log", "http", false, 0, 1, 1, 1, NULL},
    {"cgi_timeout", "http", false, 0, 1, 1, 1, isNumeric},
    {"client_timeout", "http", false, 0, 1, 1, 1, isNumeric},

    // Mime type context
    {"types", "http", true, 1, 1, 0, 0, NULL},
    {"type", "types", false, 1, static_cast<size_t>(-1), 2, static_cast<size_t>(-1), isMimeType},

    // Server context
    {"server", "http", true, 1, static_cast<size_t>(-1), 0, 0, NULL},
    {"listen", "server", false, 1, static_cast<size_t>(-1), 1, 1, isListen},
    {"server_name", "server", false, 0, static_cast<size_t>(-1), 1, static_cast<size_t>(-1), NULL},
    {"root", "server", false, 1, 1, 1, 1, NULL},
    {"index", "server", false, 0, 1, 1, static_cast<size_t>(-1), NULL},
    {"allow", "server", false, 0, static_cast<size_t>(-1), 1, static_cast<size_t>(-1), isMethod},
    {"autoindex", "server", false, 0, 1, 1, 1, isBoolean},
    {"redirect", "server", false, 0, 1, 1, 1, NULL},
    {"max_client_body_size", "server", false, 0, 1, 1, 1, isMemorySize},
    {"error_page", "server", false, 0, static_cast<size_t>(-1), 2, 2, isErrorPage},
    {"cgi", "server", false, 0, static_cast<size_t>(-1), 2, 2, isCgi},

    // Location context
    {"location", "server", true, 0, static_cast<size_t>(-1), 1, 1, isAbsolutePath},
    {"alias", "location", false, 0, 1, 1, 1, isAbsolutePath},
    {"root", "location", false, 0, 1, 1, 1, NULL},
    {"index", "location", false, 0, 1, 1, static_cast<size_t>(-1), NULL},
    {"allow", "location", false, 0, static_cast<size_t>(-1), 1, static_cast<size_t>(-1), isMethod},
    {"autoindex", "location", false, 0, 1, 1, 1, isBoolean},
    {"redirect", "location", false, 0, 1, 1, 1, NULL},
    {"max_client_body_size", "location", false, 0, 1, 1, 1, isMemorySize},
    {"cgi", "location", false, 0, static_cast<size_t>(-1), 2, 2, isCgi}};

class Config {
 private:
  std::string _path;
  std::string _includePath;
  std::string _config;

 public:
  Config();
  Config(std::string path, std::string includePath = "");
  Config(const Config &rhs);
  Config &operator=(const Config &rhs);
  ~Config();

  // Getters
  std::string getConfig() const;

  // Removes all comments from the config file
  // @exception No custom exceptions
  void removeComments();
  // Recursively parses a context
  // @param context The context to add data to
  // @param data The data to parse
  // @param line The line in the config file where the context starts
  // @return The parsed context object
  // @exception std::runtime_error If the context is invalid
  Context &parseContext(Context &context, std::string data, size_t line = 1,
                        bool validate = true);

 private:
  // Checks if the token is a valid context in the given context
  // @param context The context to check in
  // @param token The token to check
  // @return True if the token is valid in the context, false otherwise
  // @exception No custom exceptions
  bool isValidContext(Context &context, std::string token) const;

  // Checks if the token is a valid directive in the given context
  // @param context The context to check in
  // @param token The token to check
  // @return True if the token is valid in the context, false otherwise
  // @exception No custom exceptions
  bool isValidDirective(Context &context, std::string token) const;

  // Checks if the token is valid to add to the context
  // @param context The context to check in
  // @param token The token to check
  // @return An empty string if the token is valid, an error message otherwise
  // @exception No custom exceptions
  std::string validToAdd(Context &context, std::string token);

  // Checks if the arguments are valid for the given token
  // @param context The context to check in
  // @param token The token to check
  // @param args The arguments to check
  // @return An empty string if the arguments are valid, an error message
  // otherwise
  // @exception No custom exceptions
  std::string validArguments(Context &context, std::string token,
                             std::vector<std::string> args);

  // Validates the context
  // @param context The context to validate
  // @param recursive If true, the function will validate all child contexts
  // @return An empty string if the context is valid, an error message otherwise
  // @exception No custom exceptions
  std::string validateContext(Context &context, bool recursive);

  // Returns the number of lines until the given position
  // @param data The string to search in
  // @param pos The position to search until
  // @return The number of lines until the given position
  // @exception No custom exceptions
  int linesUntilPos(const std::string &data, size_t pos) const;

  // Returns the position of the end of the context
  // @param context The context to search in
  // @return The position of the end of the context
  // @exception No custom exceptions
  size_t findContextEnd(const std::string &context) const;

 private:
  // Checks if error is not empty and throws an exeption with the given message
  // and line
  void checkError(size_t line, std::string error);

  // Throws an exeption with the given message and line
  // @param line The line in the config file where the error occured
  // @param msg The message to display
  // @return std::runtime_error
  void throwExeption(size_t line, std::string msg);
};

#endif
