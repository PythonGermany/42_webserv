#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "structure.hpp"
#include "utils.hpp"

#define CONFIG_PATH "conf/server.conf"
#define LOG_LEVEL INFO
#define LOG_PATH "logs/access.log"
#define ERROR_LOG_PATH "logs/error.log"

const t_token tokens[14] = {
    {"host", "server", false, 1, 1, 1, 1, NULL},
    {"port", "server", false, 1, 1, 1, 1, isNumeric},
    {"server_name", "server", false, 0, -1, 1, -1, NULL},
    {"error_page", "server", false, 0, -1, 2, 2, NULL},
    {"client_max_body_size", "server", false, 1, 1, 1, 1, isNumeric},

    {"location", "server", true, 0, -1, 0, 0, NULL},
    {"url", "location", false, 1, 1, 1, 1, isPath},
    {"method", "location", false, 0, -1, 1, -1, isMethod},
    {"root", "location", false, 1, 1, 1, 1, isPath},
    {"index", "location", false, 1, -1, 1, -1, NULL},
    {"autoindex", "location", false, 0, 1, 1, 1, isBoolean},
    {"upload", "location", false, 0, 1, 2, 2, NULL},
    {"cgi", "location", false, 0, 1, 2, 2, NULL},
    {"redirect", "location", false, 0, 1, 1, 1, NULL}};

#endif