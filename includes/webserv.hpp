#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "structure.hpp"

#define CONFIG_PATH "server.conf"
#define LOG_LEVEL DEBUG
#define LOG_PATH "access.log"
#define ERROR_LOG_PATH "error.log"

const t_token tokens[15] = {
    {"server", "_", true, 1, -1, 0, 0},
    {"host", "server", false, 1, 1, 1, 1},
    {"port", "server", false, 1, 1, 1, 1},
    {"server_name", "server", false, 0, -1, 1, -1},
    {"error_page", "server", false, 0, -1, 2, 2},
    {"client_max_body_size", "server", false, 1, 1, 1, 1},

    {"location", "server", true, 1, -1, 0, 0},
    {"url", "location", false, 1, 1, 1, 1},
    {"method", "location", false, 0, -1, 1, -1},
    {"root", "location", false, 1, 1, 1, 1},
    {"index", "location", false, 1, -1, 1, -1},
    {"autoindex", "location", false, 0, 1, 1, 1},
    {"upload", "location", false, 0, 1, 1, 1},
    {"cgi", "location", false, 0, 1, 2, 2},
    {"redirect", "location", false, 0, 1, 1, 1}};

#endif