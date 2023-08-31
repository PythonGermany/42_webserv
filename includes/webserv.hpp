#ifndef WEBSERV_HPP
#define WEBSERV_HPP

// Holds the default values and the
// general structure of the config file

#include "structure.hpp"
#include "utils.hpp"

#define CONFIG_PATH "/etc/webserv/server.conf"
#define LOG_LEVEL INFO
#define LOG_PATH "/var/log/webserv/access.log"
#define ERROR_LOG_PATH "/var/log/webserv/error.log"

const t_token tokens[22] = {
    {"log_level", "_", false, 0, 1, 1, 1, isLogLevel},
    {"access_log", "_", false, 0, 1, 1, 1, isAbsolutePath},
    {"error_log", "_", false, 0, 1, 1, 1, isAbsolutePath},

    {"server", "_", true, 1, -1, 0, 0, NULL},
    {"host", "server", false, 1, 1, 1, 1, NULL},
    {"port", "server", false, 1, 1, 1, 1, isNumeric},
    {"server_name", "server", false, 0, -1, 1, -1, NULL},
    {"client_max_body_size", "server", false, 1, 1, 1, 1, isNumeric},

    {"error_page", "server", true, 0, -1, 0, 0, NULL},
    {"code", "error_page", false, 1, 1, 1, -1, isNumeric},
    {"path", "error_page", false, 1, 1, 1, 1, isAbsolutePath},

    {"location", "server", true, 0, -1, 0, 0, NULL},
    {"url", "location", false, 1, 1, 1, 1, isAbsolutePath},
    {"method", "location", false, 0, -1, 1, -1, isMethod},
    {"root", "location", false, 1, 1, 1, 1, isAbsolutePath},
    {"index", "location", false, 1, -1, 1, -1, NULL},
    {"autoindex", "location", false, 0, 1, 1, 1, isBoolean},
    {"upload", "location", false, 0, 1, 1, 1, isAbsolutePath},
    {"redirect", "location", false, 0, 1, 1, 1, NULL},

    {"cgi", "location", true, 0, 1, 0, 0, NULL},
    {"extension", "cgi", false, 1, 1, 1, 1, NULL},
    {"path", "cgi", false, 1, 1, 1, 1, isAbsolutePath}};

#endif