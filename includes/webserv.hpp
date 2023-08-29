#ifndef WEBSERV_HPP
#define WEBSERV_HPP

typedef enum e_log_level { DEBUG, INFO, WARNING } t_log_level;

#define CONFIG_PATH "server.conf"

#define LOG_LEVEL DEBUG
#define LOG_PATH "access.log"
#define ERROR_LOG_PATH "error.log"

#include "colors.hpp"

#endif