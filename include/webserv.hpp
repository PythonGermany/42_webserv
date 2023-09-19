#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// Holds the default values and the
// general structure of the config file

#include "config/structure.hpp"
#include "config/utils.hpp"

#define CONFIG_PATH "/etc/webserv/webserv.conf"
#define LOG_LEVEL INFO
#define LOG_PATH "/var/log/webserv/access.log"
#define ERROR_LOG_PATH "/var/log/webserv/error.log"
#define TIMEOUT 10000
#define CLIENT_MAX_BODY_SIZE 1048576

// Token structure in the format: {name, parent, isContext, minOccurence,
// maxOccurence, minArgs, maxArgs, validationFunction}
const t_token tokens[28] = {
    {"http", "_", true, 1, 1, 0, 0, NULL},
    {"log_level", "http", false, 0, 1, 1, 1, isLogLevel},
    {"access_log", "http", false, 0, 1, 1, 1, NULL},
    {"error_log", "http", false, 0, 1, 1, 1, NULL},
    {"timeout", "http", false, 0, 1, 1, 1, isNumeric},

    // Mime type context
    {"types", "http", true, 1, 1, 0, 0, NULL},
    {"type", "types", true, 1, -1, 0, 0, NULL},
    {"mime", "type", false, 1, 1, 1, 1, NULL},
    {"extension", "type", false, 1, 1, 1, -1, isExtension},

    // Server context
    {"server", "http", true, 1, -1, 0, 0, NULL},
    {"listen", "server", false, 1, -1, 1, 1, isListen},
    {"server_name", "server", false, 0, -1, 1, -1, NULL},
    {"client_max_body_size", "server", false, 1, 1, 1, 1, isNumeric},
    // Error page context
    {"error_page", "server", true, 0, -1, 0, 0, NULL},
    {"code", "error_page", false, 1, 1, 1, -1, isNumeric},
    {"path", "error_page", false, 1, 1, 1, 1, NULL},
    // Location context
    {"location", "server", true, 0, -1, 0, 0, NULL},
    {"url", "location", false, 1, 1, 1, 1, isAbsolutePath},
    {"method", "location", false, 0, -1, 1, -1, isMethod},
    {"root", "location", false, 1, 1, 1, 1, NULL},
    {"index", "location", false, 1, -1, 1, -1, NULL},
    {"autoindex", "location", false, 0, 1, 1, 1, isBoolean},
    {"upload", "location", false, 0, 1, 1, 1, NULL},
    {"redirect", "location", false, 0, 1, 1, 1, NULL},
    // CGI context
    {"cgi", "location", true, 0, 1, 0, 0, NULL},
    {"extension", "cgi", false, 1, 1, 1, 1, isExtension},
    {"path", "cgi", false, 1, 1, 1, 1, NULL}};

#endif