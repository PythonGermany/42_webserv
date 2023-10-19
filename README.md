[![ubuntu](https://github.com/PythonGermany/42_webserv/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/ubuntu.yml)
[![macos](https://github.com/PythonGermany/42_webserv/actions/workflows/macos.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/macos.yml)

# TODO

- [ ] Needed? -> Implement so that and alias updating an uri also updates context
- [ ] Investigate NS_ERROR_NET_RESET error for put request 404 and 413 response
- [ ] Figure out why resolving symlinks doesnt work

- [ ] (Does not occur anymore without having actively tried to fix it) Investigate random closing of program without any notice after sending a first request

- [x] Replace c function for wildcard pattern matching with own function
- [x] Figure out why login for wordpress doesn't work
- [x] Implement cgi support
- [x] Fix issue of cases like /uploaded/../listing/index.php where restrictions of one context can be bypassed
- [x] Implement resolving of . and .. for request uri (also updates context accordingly since this will happen before context selection)
- [x] Fix file uploading
- [x] Implement support for multilistendirective or change config
- [x] Implement chunked transfer encoding (https://datatracker.ietf.org/doc/html/rfc2616#section-3.6 
                                          && https://datatracker.ietf.org/doc/html/rfc2616#section-19.4.6)

- [ ] <del>Maybe use streambufs instead of streams https://gcc.gnu.org/onlinedocs/libstdc++/manual/fstreams.html#std.io.filestreams.binary</del>
- [ ] <del>Maybe implement configurable default mime</del>

- [ ] Remove unwanted submodules and config once ready for evaluation

# Contents

- [Introduction](#introduction)
- [Linux installation](#linux-installation)
  - [Requirements](#requirements)
  - [Compilation](#compilation)
- [Usage](#usage)
  - [Flags](#flags)
- [Configuration](#configuration)
  - [Contexts](#contexts)
  - [Directives](#directives)
  - [Example](#example)

# Introduction

This project is a basic HTTP server written in C++98.

| Functionality | Description | External info
| --- | --- | --- | 
| HTTP | HTTP/1.1 | [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616) |
| CGI | CGI/1.1 (Tested with php-cgi, using wordpress, and python cgi) | [RFC 3875](https://datatracker.ietf.org/doc/html/rfc3875) |
| Implemented methods | GET / HEAD / OPTIONS / POST / PUT / DELETE | [RFC 2616 section 5.1.1](https://datatracker.ietf.org/doc/html/rfc2616#section-5.1.1) |
| Basic cookie support | Tested with wordpress | [RFC 2965](https://datatracker.ietf.org/doc/html/rfc2965) | # TODO: RFC right external info?
| Argument flags | [To description](#flags) | - |
| Configuration file | [To description](#configuration) | - |

# Linux installation

## Requirements

| Requirement | Installation |
| --- | --- |
| C++98 | sudo apt-get install clang |
| Make | sudo apt-get install make |

## Compilation

```
make [debug]
```

# Usage
```
./webserv [configuration_file] [ [-i|-h|-v|-c|-t] | [-o|-l|-a|-e ARGUMENT] ...]
```
Default configuration file: `/etc/webserv/webserv.conf`

## Flags

A flag overwrites the default setting as well as the ones that are configured in the config file.

| Flag | Description | Allowed values
| --- | --- | --- |
| i | Show info block when starting the server | NONE |
| h | Show this help message and exit | NONE |
| v | Show version of webserv and exit | NONE
| c | Show parsed config file structure and exit | NONE |
| t | Check if the config file syntax is valid and exit | NONE |
| o | Controls [log_to_terminal](#log_to_terminal) | on/off |
| l | Controls [log_level](#log_level) | 0 / 1 / 2 / 3 / 4 for `error` / `warning` / `info` / `debug` / `verbose`
| a | Controls [access_log](#access_log) | PATH
| e | Controls [error_log](#error_log) | PATH

**Flags can be utilized to configure an option directly from the start of the program instead of only after the configuration has been read and parsed.**  

### Example:
```
./webserv [configuration_file] -s on -l 3 -a PATH
```
You can look at the parsing debug output by setting the flag `s` to `on` and the flag `l` to `3` or higher, this will override the default setting before the config has been read, parsed and applied. If you also want to write the parsing log messages to a file the flag `a` needs to be set to the `path` you want the log file to be. Those steps are necessary because even if you specify all the previous flag settings in the config file they will only be applied after having successfully loaded your config settings, which will, of course, happen after the config file has been parsed.

# Configuration

The configuration file is used to define the global configuration of the webserver. It is composed of multiple contexts. Each context can contain multiple directives. A directive is composed of a name arguments and a value. The value can be a string, a number or a block. A block is a list of directives.

| Config type | Options |
| --- | --- |
| Configuration file context | [http](#http) / [location](#location) / [server](#server) |
| Configuration file directives | [access_log](#access_log) / [alias](#alias) / [allow](#allow) / [autoindex](#autoindex) / [cgi](#cgi) / [error_log](#error_log) / [error_page](#error_page) / [include](#include) / [index](#index) / [listen](#listen) / [log_level](#log_level) / [log_to_terminal](#log_to_terminal) / [max_client_body_size](#max_client_body_size)  / [redirect](#redirect) / [root](#root) / [server_name](#server_name) / [type](#type) |

## Contexts

### Http
```nginx
http {
    types {
        [directives]
    }

    include PATH;
    log_to_terminal [on|off];
    log_level LEVEL;
    access_log PATH;
    error_log PATH;

    server {
        [directives]
    }
}
```
Root context. It contains the global configuration of the webserver.  
**Allowed tokens:** [access_log](#access_log) / [error_log](#error_log) / [include](#include) / [log_level](#log_level) / [log_to_terminal](#log_to_terminal) / [server](#server) / [types](#types)

### Types
```nginx
types {
  type MIME_TYPE EXTENSION [EXTENSION ...];
}
```
Types context. It contains the mime types of the server.  
**Allowed tokens:** [type](#type)

### Server
```nginx
server {
    listen HOST:PORT;
    server_name NAME [NAME ...];
    root PATH;
    index FILE [FILE ...];

    autoindex [on|off];
    max_client_body_size SIZE[k|m];
    allow METHOD [METHOD ...];
    error_page CODE PATH;
    cgi EXTENSION PATH;

    location PATH {
        [directives]
    }
}
```
Virtual server context. It contains the configuration of a virtual server.  
**Allowed tokens:** [allow](#allow) / [autoindex](#autoindex) / [cgi](#cgi) / [error_page](#error_page) / [index](#index) / [listen](#listen) / [location](#location) / [max_client_body_size](#max_client_body_size) / [root](#root) / [server_name](#server_name)

### Location
```nginx
location PATH {
    alias PATH;
    root PATH;
    index FILE [FILE ...];
    
    allow METHOD [METHOD ...];
    autoindex [on|off];
    redirect URL;
    max_client_body_size SIZE;
    cgi EXTENSTION PATH;
}
```
Location context for `PATH`. It contains the configuration of a location.  
**Allowed tokens:** [alias](#alias) / [allow](#allow) / [autoindex](#autoindex) / [cgi](#cgi) / [index](#index) / [max_client_body_size](#max_client_body_size) / [redirect](#redirect) / [root](#root) 

## Directives

### access_log
```nginx
access_log PATH;
```
Sets the path of the access log file.  
Default: `/var/log/webserv/access.log`  
**Allowed in:** [Http](#http)

### alias
```nginx
alias PATH;
```
Set an alias path. Example request for alias PATH: `GET /alias/file` -> `root/PATH/file`  
**Allowed in:** [Location](#location)

### allow
```nginx
allow METHOD [METHOD ...];
```
Sets the allowed methods for a context.  
Default: `GET` / `HEAD` / `OPTIONS`  
**Allowed in:** [Http](#http) / [Location](#location)

### autoindex
```nginx
autoindex [on|off];
```
Enables or disables the directory listing for a context  
Default: `off`  
**Allowed in:** [Http](#http) / [Location](#location)

### cgi
```nginx
cgi EXTENSTION PATH;
```
Sets the cgi EXECUTABLE to path for EXTENSION.    
**Allowed in:** [Location](#location) / [Server](#server)

### error_log
```nginx
error_log PATH;
```
Sets the path of the error log file.  
Default: `/var/log/webserv/error.log`  
**Allowed in:** [Http](#http)

### error_page
```nginx
error_page CODE PATH;
```
Sets a custom error page for the given status code.  
**Allowed in:** [Server](#server)

### include
```nginx
include PATH;
```
Include other configuration files. Supports `*` wildcard character
**Allowed in:** [Cgi](#cgi) / [Http](#http) / [Location](#location) / [Server](#server)

### index
```nginx
index FILE [FILE ...];
```
Sets the index files for a context  
**Allowed in:** [Location](#location) / [Server](#server)

### listen
```nginx
listen HOST:PORT;
```
Sets the host and port of the server.  
**Allowed in:** [Server](#server)

### log_level
```nginx
log_level LEVEL;
```
Set the log level. Allowed log levels are `debug`, `info`, `warning`, `error` and `verbose`.  
Default: `info`  
**Allowed in:** [Http](#http)

### log_to_terminal
```nginx
log_to_terminal [on|off];
```
Sets whether logs should also be displayed on the terminal.  
Default: `off`  
**Allowed in:** [Http](#http)

### max_client_body_size
```nginx
max_client_body_size SIZE[k|m];
```
Sets the maximum body size for a client request message.  
Default: `1m`  
**Allowed in:** [Location](#location) / [Server](#server)

### redirect
```nginx
redirect URL;
```
Redirects the request of the context to the given url.   
**Allowed in:** [Location](#location) / [Server](#server)

### root
```nginx
root PATH;
```
Sets the root path for a context.  
**Allowed in:** [Location](#location) / [Server](#server)

### server_name
```nginx
server_name NAME [NAME ...];
```
Sets the server names.  
**Allowed in:** [Server](#server)

### type
```nginx
type MIME_TYPE EXTENSION [EXTENSION ...];
```
Set the mime type for the given extensions.  
**Allowed in:** [Types](#types)

## Example
```nginx
# File -> /etc/webserv/mime.types
types {
  type text/html html htm;
  type text/css css;
  type text/javascript js;
  type image/jpeg jpeg jpg;
  type image/png png;
  type image/svg+xml svg;
  type image/gif gif;
}
```
```nginx
# File -> /etc/webserv/webserv.conf
http {
  include /etc/webserv/mime.types;

  log_to_terminal off;
  log_level info;
  access_log /var/log/webserv/access.log;
  error_log /var/log/webserv/error.log;

  include /etc/webserv/sites-enabled/*;
}
```
```nginx
# File -> /etc/webserv/sites-enabled/server.conf
server {
  listen 8080;
  root /var/www/html;
  index index.html index.htm;
  autoindex on;
  max_client_body_size 1m;
  allow GET HEAD OPTIONS;
  error_page 404 /404.html;
  cgi php /usr/bin/php-cgi;

  location /example {
    alias /www; # Request: GET /example/file -> ROOT/www/file
    index example.html;
    allow GET HEAD OPTIONS PUT DELETE;
    autoindex off;
  }
  location /redirect {
    redirect https://www.duckduckgo.com;
  }
}
```
```
./webserv [-i]
```
