#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// Holds the default values and the
// general structure of the config file

#include "config/structure.hpp"
#include "config/utils.hpp"

// Default values for program
#define CONFIG_PATH "/etc/webserv/webserv.conf"

// Default values for log class
#define LOG_LEVEL INFO
#define LOG_PATH "/var/log/webserv/access.log"
#define ERROR_LOG_PATH "/var/log/webserv/error.log"
#define LOG_TIME_FORMAT "%H:%M:%S GMT"
#define LOG_DATE_FORMAT "%d-%m-%Y"

// Default values for http class
#define HTTP_VERSION "HTTP/1.1"
#define HTTP_METHOD_COUNT 6
#define HTTP_METHODS \
  { "GET", "HEAD", "OPTIONS", "POST", "PUT", "DELETE" }
#define HTTP_DEFAULT_METHOD_COUNT 3
#define HTTP_DEFAULT_METHODS \
  { "GET", "HEAD", "OPTIONS" }
#define MAX_CLIENT_BODY_SIZE 1048576

// Default values for cache class
#define CACHE_DATA_LIFETIME 60            // seconds
#define CACHE_MAX_SIZE 256 * 1024 * 1024  // 512 MB

// Default values for poll class
/**
 * close connections if they are TIMEOUT milliseconds inactive
 */
#define TIMEOUT 60000

// Token structure in the format: {name, parent, isContext, minOccurence,
// maxOccurence, minArgs, maxArgs, validationFunction}
const t_token tokens[30] = {
    {"http", "_", true, 1, 1, 0, 0, NULL},
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
    {"listen", "server", false, 1, -1, 1, 1, isListen},
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

#endif