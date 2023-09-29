#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// Holds the default values and the
// general structure of the config file

#include "config/utils.hpp"
#include "structure.hpp"

// -------------------------- ARG VALUES ---------------------------

// Allowd argument input structure in the format: {flag, argCount,
// validation-/initializationFunction}
const arg_t args_g[6] = {{'i', 0, setInfo},        {'h', 0, setHelp},
                         {'s', 1, setLogToStdout}, {'l', 1, setLogLevel},
                         {'a', 1, setAccessLog},   {'e', 1, setErrorLog}};

// ------------------------- CONFIG VALUES -------------------------

#define CONFIG_PATH "/etc/webserv/webserv.conf"

// Allowed token input structure in the format: {name, parent, isContext,
// minOccurence, maxOccurence, minArgs, maxArgs, validationFunction}
const token_t tokens[31] = {
    {"http", "_", true, 1, 1, 0, 0, NULL},
    {"log_to_stdout", "http", false, 0, 1, 1, 1, isBoolean},
    {"log_level", "http", false, 0, 1, 1, 1, isLogLevel},
    {"access_log", "http", false, 0, 1, 1, 1, NULL},
    {"error_log", "http", false, 0, 1, 1, 1, NULL},
    {"timeout", "http", false, 0, 1, 1, 1,
     isNumeric},  // TODO: Implement or remove

    // Mime type context
    {"types", "http", true, 1, 1, 0, 0, NULL},
    {"type", "types", false, 1, -1, 2, -1, isMimeType},

    // Server context
    {"server", "http", true, 1, -1, 0, 0, NULL},
    {"listen", "server", false, 1, -1, 1, 1,
     isListen},  // TODO: Implement support for multilistendirective or change
                 // config
    {"server_name", "server", false, 0, -1, 1, -1, NULL},
    {"root", "server", false, 1, 1, 1, 1, NULL},
    {"index", "server", false, 0, 1, 1, -1, NULL},
    {"allow", "server", false, 0, -1, 1, -1, isMethod},
    {"autoindex", "server", false, 0, 1, 1, 1, isBoolean},
    {"redirect", "server", false, 0, 1, 1, 1, NULL},
    {"max_client_body_size", "server", false, 0, 1, 1, 1, isNumeric},
    {"error_page", "server", false, 0, -1, 2, 2, isErrorPage},
    // Location context
    {"location", "server", true, 0, -1, 1, 1, isAbsolutePath},
    {"alias", "location", false, 0, 1, 1, 1, isAbsolutePath},
    {"root", "location", false, 0, 1, 1, 1, NULL},
    {"index", "location", false, 0, 1, 1, -1, NULL},
    {"allow", "location", false, 0, -1, 1, -1, isMethod},
    {"autoindex", "location", false, 0, 1, 1, 1, isBoolean},
    {"redirect", "location", false, 0, 1, 1, 1, NULL},
    {"max_client_body_size", "location", false, 0, 1, 1, 1, isNumeric},
    // CGI context
    {"cgi", "location", true, 0, 1, 1, 1, isExtension},
    {"cgi_path", "cgi", false, 1, 1, 1, 1, NULL}};

// -------------------------- LOG VALUES ---------------------------

#define LOG_TO_STDOUT false
#define LOG_LEVEL INFO
#define LOG_STDOUT_OVERRIDE_LEVEL ERROR
#define LOG_PATH "/var/log/webserv/access.log"
#define LOG_ERROR_PATH "/var/log/webserv/error.log"
#define LOG_TIME_FORMAT "%H:%M:%S GMT"
#define LOG_DATE_FORMAT "%d-%m-%Y"

// -------------------------- HTTP VALUES --------------------------

#define HTTP_VERSION "HTTP/1.1"
#define HTTP_METHOD_COUNT 6
#define HTTP_METHODS \
  { "GET", "HEAD", "OPTIONS", "POST", "PUT", "DELETE" }
#define HTTP_DEFAULT_METHOD_COUNT 3
#define HTTP_DEFAULT_METHODS \
  { "GET", "HEAD", "OPTIONS" }
#define HTTP_DEFAULT_MIME "application/octet-stream"
#define MAX_CLIENT_BODY_SIZE 1048576
// Indent size for http multiline log output
#define INDENT "\r\n                          "

// -------------------------- POLL VALUES ---------------------------

/**
 * close connections if they are TIMEOUT milliseconds inactive
 */
#define TIMEOUT 30000

// ----------------------- ADDITIONAL VALUES -----------------------

// Default buffer size for various different classes
// like AConnection  Http and ResponsePipe
#define BUFFER_SIZE 65536

#endif