#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// Holds the default values and the
// general structure of the config file

#include "config/structure.hpp"
#include "config/utils.hpp"

#define CONFIG_PATH "/etc/webserv/webserv.conf"

#define LOG_LEVEL DEBUG
#define LOG_PATH "/var/log/webserv/access.log"
#define ERROR_LOG_PATH "/var/log/webserv/error.log"
#define LOG_TIME_FORMAT "%H:%M:%S GMT"
#define LOG_DATE_FORMAT "%d-%m-%Y"

/**
 * close connections if they are TIMEOUT milliseconds inactive
 */
#define TIMEOUT 100000

#define MAX_CLIENT_BODY_SIZE 1048576

// Token structure in the format: {name, parent, isContext, minOccurence,
// maxOccurence, minArgs, maxArgs, validationFunction}
const t_token tokens[34] = {
    {"http", "_", true, 1, 1, 0, 0, NULL},
    {"log_level", "http", false, 0, 1, 1, 1, isLogLevel},
    {"access_log", "http", false, 0, 1, 1, 1, NULL},
    {"error_log", "http", false, 0, 1, 1, 1, NULL},
    {"timeout", "http", false, 0, 1, 1, 1,
     isNumeric},  // TODO: Implement or remove

    // Mime type context
    {"types", "http", true, 1, 1, 0, 0, NULL},
    {"type", "types", true, 1, -1, 1, 1, NULL},
    {"extension", "type", false, 1, 1, 1, -1, isExtension},

    // Server context
    {"server", "http", true, 1, -1, 0, 0, NULL},
    {"listen", "server", false, 1, -1, 1, 1, isListen},
    {"server_name", "server", false, 0, -1, 1, -1, NULL},
    {"root", "server", false, 1, 1, 1, 1, NULL},
    {"index", "server", false, 0, 1, 1, -1, NULL},
    {"allow", "server", false, 0, -1, 1, -1, isMethod},
    {"autoindex", "server", false, 0, 1, 1, 1, isBoolean},
    {"upload", "server", false, 0, 1, 1, 1, isAbsolutePath},
    {"redirect", "server", false, 0, 1, 1, 1, NULL},
    {"max_client_body_size", "server", false, 0, 1, 1, 1, isNumeric},
    // Error page context
    {"error_page", "server", true, 0, -1, 1, 1, isNumeric},
    {"path", "error_page", false, 1, 1, 1, 1, isAbsolutePath},
    // Location context
    {"location", "server", true, 0, -1, 1, 1, isAbsolutePath},
    {"alias", "location", false, 0, 1, 1, 1, NULL},
    {"root", "location", false, 0, 1, 1, 1, NULL},
    {"index", "location", false, 0, 1, 1, -1, NULL},
    {"allow", "location", false, 0, -1, 1, -1, isMethod},
    {"autoindex", "location", false, 0, 1, 1, 1, isBoolean},
    {"upload", "location", false, 0, 1, 1, 1, isAbsolutePath},
    {"redirect", "location", false, 0, 1, 1, 1, NULL},
    {"max_client_body_size", "location", false, 0, 1, 1, 1, isNumeric},
    // CGI context
    {"cgi", "location", true, 0, 1, 1, 1, isExtension},
    {"path", "cgi", false, 1, 1, 1, 1, NULL}};

#endif