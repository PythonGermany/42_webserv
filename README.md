[![ubuntu](https://github.com/PythonGermany/42_webserv/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/ubuntu.yml)
[![macos](https://github.com/PythonGermany/42_webserv/actions/workflows/macos.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/macos.yml)

HTTP RFC: https://datatracker.ietf.org/doc/html/rfc2616  
Cookie RFC: https://datatracker.ietf.org/doc/html/rfc2965  
CGI RFC: https://datatracker.ietf.org/doc/html/rfc3875

# TODO

- [ ] Investigate random closing of program without any notice after sending a first request
- [ ] Investigate NS_ERROR_NET_RESET error for put request 404 response
- [ ] <del>Maybe implement configurable default mime</del>
- [x] Fix file uploading
- [ ] <del>Maybe use streambufs instead of streams https://gcc.gnu.org/onlinedocs/libstdc++/manual/fstreams.html#std.io.filestreams.binary</del>
- [x] Implement support for multilistendirective or change config
- [ ] Figure out why resolving symlinks doesnt work
- [x] Implement chunked transfer encoding (https://datatracker.ietf.org/doc/html/rfc2616#section-3.6 
                                          && https://datatracker.ietf.org/doc/html/rfc2616#section-19.4.6)
- [ ] Implement cgi support

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

This project is a webserver written in C++98. It's functionality is listed below:

| Functionality | Description |
| --- | --- |
| Http | Http/1.1 |
| Methods | GET / HEAD / OPTIONS / POST / PUT / DELETE |
| Logs | Access log / Error log |
| Configuration file context | [cgi](#cgi) / [http](#http) / [location](#location) / [server](#server) |
| Configuration file directives | [access_log](#access_log) / [alias](#alias) / [allow](#allow) / [autoindex](#autoindex) / [cgi_path](#cgi_path) / [error_log](#error_log) / [error_page](#error_page) / [include](#include) / [index](#index) / [listen](#listen) / [log_level](#log_level) / [log_to_stdout](#log_to_stdout) / [max_client_body_size](#max_client_body_size)  / [redirect](#redirect) / [root](#root) / [server_name](#server_name) / [type](#type) |
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
./webserv [configuration_file] [-i|-h] [ [-FLAG ARGUMENT] ...]
```
Default configuration file: `/etc/webserv/webserv.conf`

## Flags
A flag overwrites the default setting as well as one that may be configured in the config file. It can be utilized to configure an option directly from the start of the program instead of only after the configuration has been read and parsed.

| Flag | Description | Allowed values
| --- | --- | --- |
| i | Prints out some information before starting the server | NONE |
| h | Prints out some help before starting the server | NONE |
| s | Controls [log_to_stdout](#log_to_stdout) | on/off |
| l | Controls [log_level](#log_level) | 0 / 1 / 2 / 3 / 4 for `error` / `warning` / `info` / `debug` / `verbose`
| a | Controls [access_log](#access_log) | PATH
| e | Controls [error_log](#error_log) | PATH

# Configuration

The configuration file is used to define the global configuration of the webserver. It is composed of multiple contexts. Each context can contain multiple directives. A directive is composed of a name arguments and a value. The value can be a string, a number or a block. A block is a list of directives.

## Contexts

### Http
```nginx
http {
    types {
        [directives]
    }

    include PATH;
    log_to_stdout [on|off];
    log_level LEVEL;
    access_log PATH;
    error_log PATH;

    server {
        [directives]
    }
}
```
Root context. It contains the global configuration of the webserver.  
**Allowed tokens:** [access_log](#access_log) / [error_log](#error_log) / [include](#include) / [log_level](#log_level) / [log_to_stdout](#log_to_stdout) / [server](#server) / [types](#types)

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
    max_client_body_size SIZE;
    allow METHOD [METHOD ...];
    error_page CODE PATH;

    location PATH {
        [directives]
    }
}
```
Virtual server context. It contains the configuration of a virtual server.  
**Allowed tokens:** [allow](#allow) / [autoindex](#autoindex) / [error_page](#error_page) / [index](#index) / [listen](#listen) / [location](#location) / [max_client_body_size](#max_client_body_size) / [root](#root) / [server_name](#server_name)

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

    cgi EXTENSION {
        [directives];
    }
}
```
Location context for `PATH`. It contains the configuration of a location.  
**Allowed tokens:** [alias](#alias) / [allow](#allow) / [autoindex](#autoindex) / [cgi](#cgi) / [index](#index) / [max_client_body_size](#max_client_body_size) / [redirect](#redirect) / [root](#root) 

### Cgi
```nginx
cgi EXTENSION {
    cgi_path PATH;
}
```
Cgi context for `EXTENSION`. It contains the path of the cgi file.  
**Allowed tokens:** [cgi_path](#cgi_path)

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

### cgi_path
```nginx
cgi_path PATH;
```
Sets the executable for the cgi context.    
**Allowed in:** [Cgi](#cgi)

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
Include another configuration file.  
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

### log_to_stdout
```nginx
log_to_stdout [on|off];
```
Sets whether logs should also be displayed on the standard output.  
Default: `off`  
**Allowed in:** [Http](#http)

### max_client_body_size
```nginx
max_client_body_size SIZE;
```
Sets the maximum body size for a client request message.  
Default: `1048576`  
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

  log_to_stdout off;
  log_level info;
  access_log /var/log/webserv/access.log;
  error_log /var/log/webserv/error.log;

  include /etc/webserv/sites-enabled/*;
}
```
```nginx
# File -> /etc/webserv/sites-enabled/server.conf
server {
  listen localhost:8080;
  server_name localhost:8080;
  root /var/www/html;
  index index.html index.htm;
  autoindex on;
  max_client_body_size 1048576;
  allow GET HEAD OPTIONS;

  error_page 404 /404.html;

  location /example {
    alias /www; # Request: GET /example/file -> root/www/file
    index example.html;
    allow GET HEAD OPTIONS PUT DELETE;
    autoindex off;
  }
  location /redirect {
    redirect http://www.duckduckgo.com;
  }
  location /cgi {
    cgi php {
      cgi_path /usr/bin/php-cgi;
    }
  }
}
```
```
./webserv -i
```