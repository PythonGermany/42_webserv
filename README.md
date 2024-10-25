[![Format check](https://github.com/PythonGermany/42_webserv/actions/workflows/format-check.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/format-check.yml)
[![Linux build](https://github.com/PythonGermany/42_webserv/actions/workflows/linux-build.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/linux-build.yml)
[![Docker ci](https://github.com/PythonGermany/42_webserv/actions/workflows/docker-ci.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/docker-ci.yml)

# Introduction

This project is a basic HTTP server written in C++98. The standards listed below (HTTP/CGI) are not completely implemented and were rather seen as a guideline to implement a usable server than a "to be followed requirement".

| Functionality | Description | External info
| --- | --- | --- | 
| HTTP | HTTP/1.1 | [RFC 2616](https://datatracker.ietf.org/doc/html/rfc2616) / [RFC 9110](https://datatracker.ietf.org/doc/html/rfc9110#section-9.3.7) / [RFC 9112](https://datatracker.ietf.org/doc/html/rfc9112#name-chunked-transfer-coding) |
| CGI | CGI/1.1 (Tested with php-cgi, using wordpress, and python cgi) | [RFC 3875](https://datatracker.ietf.org/doc/html/rfc3875) |
| Implemented methods | GET / HEAD / OPTIONS / POST / PUT / DELETE | [RFC 2616 section 5.1.1](https://datatracker.ietf.org/doc/html/rfc2616#section-5.1.1) |
| Basic cookie support | Tested with wordpress | [RFC 2109](https://datatracker.ietf.org/doc/html/rfc2109) |
| Argument flags | [To description](#flags) | - |
| Configuration file | [To description](#configuration) | - |

# Table of Contents

- [Linux installation](#linux-installation)
  - [Requirements](#requirements)
  - [Compilation](#compilation)
- [Usage](#usage)
  - [Flags](#flags)
- [Configuration](#configuration)
  - [Contexts](#contexts)
  - [Directives](#directives)
  - [Example](#example)

# Linux installation

## Requirements

[Docker](https://docs.docker.com/engine/install/) or

```
apt install make
```

```
apt install clang
```

## Compilation

### Native

```
make [performance|debug|fsanitize]
```

### Docker

This will create a mariadb docker container for the database another container which will setup wordpress, compile webserv execute it. To populate the container env credentials you can make a copy of the [.env-example](docker/.env-example) file and rename it to `.env`, make sure to change the default values according to your needs.
```
make -C docker up
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
| h | Show help message and exit | NONE |
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
./webserv [configuration_file] -s on -l 2 -a PATH
```
You can look at the parsing debug output by setting the flag `o` to `on` and the flag `l` to `3` or higher, this will override the default setting before the config has been read, parsed and applied. If you also want to write the parsing log messages to a file the flag `a` needs to be set to the `PATH` you want the log file to be. Those steps are necessary because even if you specify all the previous flag settings in the config file they will only be applied after having successfully loaded your config settings, which will, of course, happen after the config file has been parsed.

# Configuration

The configuration file is used to define the configuration of the webserver. It is composed of directives. A directive is composed of a name arguments and a value. The value can be a string or a block. A block is a list of directives. The file [webserv.conf](conf/webserv.conf) demonstrates the default configuration of the webserv executable.

| Config type | Options |
| --- | --- |
| Configuration file context | [http](#http) / [location](#location) / [server](#server) |
| Configuration file directives | [access_log](#access_log) / [alias](#alias) / [allow](#allow) / [autoindex](#autoindex) / [cgi](#cgi) / [cgi_timeout](#cgi_timeout) / [client_timeout](#client_timout) / [error_log](#error_log) / [error_page](#error_page) / [include](#include) / [index](#index) / [listen](#listen) / [log_level](#log_level) / [log_to_terminal](#log_to_terminal) / [max_client_body_size](#max_client_body_size)  / [redirect](#redirect) / [root](#root) / [server_name](#server_name) / [type](#type) |

## Contexts

### Http
```nginx
http {
  include PATH;

  types {
    [directives]
  }

  cgi_timeout NUMBER;
  client_timeout NUMBER;

  access_log PATH;
  error_log PATH;
  log_to_terminal [on|off];
  log_level LEVEL;

  server {
    [directives]
  }
}
```
Root context. It contains the global configuration of the webserver.  
**Allowed tokens:** [access_log](#access_log) / [cgi_timeout](#cgi_timeout) / [client_timeout](#client_timout)/ [error_log](#error_log) / [include](#include) / [log_level](#log_level) / [log_to_terminal](#log_to_terminal) / [server](#server) / [types](#types)

### Types
```nginx
types {
  type MIME_TYPE EXTENSION [EXTENSION ...];
}
```
Types context. It contains the mime types of the server. The file [mime.types](conf/mime.types) provides an extensive default configuration for this context.  
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
Set an alias path. Example request for alias PATH: `GET /LOCATION_CONTEXT_PATH/file` -> `ROOT/PATH/file`  
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

### cgi_timeout
```nginx
cgi_timeout NUMBER;
```
Sets the cgi timeout in milliseconds.  
Default: `30000`  
**Allowed in:** [Http](#http)

### client_timout
```nginx
client_timout NUMBER;
```
Sets the client timeout in milliseconds.  
Default: `30000`  
**Allowed in:** [Http](#http)

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
Sets the maximum body size for a client request message. 1k is 1024 bytes, 1m is 1024k.  
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

  cgi_timeout 30000;
  client_timeout 30000;

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
