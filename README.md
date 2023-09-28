[![ubuntu](https://github.com/PythonGermany/42_webserv/actions/workflows/ubuntu.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/ubuntu.yml)
[![macos](https://github.com/PythonGermany/42_webserv/actions/workflows/macos.yml/badge.svg)](https://github.com/PythonGermany/42_webserv/actions/workflows/macos.yml)

# TODO

- [ ] Figure out why log file cant be opened on linode when being customized
- [ ] Fix file uploading
- [ ] Maybe use streambufs instead of streams https://gcc.gnu.org/onlinedocs/libstdc++/manual/fstreams.html#std.io.filestreams.binary
- [ ] Figure out why resolving symlinks doesnt work
- [ ] Implement chunked transfer encoding (https://datatracker.ietf.org/doc/html/rfc2616#section-3.6 
                                          && https://datatracker.ietf.org/doc/html/rfc2616#section-19.4.6)
- [ ] Implement cgi support

- [ ] Remove unwanted submodules once ready for evaluation

# Contents

- [42_webserv](#42_webserv)
  - [Contents](#contents)
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
| Configuration file context | [http](#http) / [server](#server) / [location](#location) / [cgi](#cgi) |
| Configuration file directives | [type](#type) / [include](#include) / [log_level](#log_level) / [access_log](#access_log) / [error_log](#error_log) / [listen](#listen) / [server_name](#server_name) / [root](#root) / [index](#index) / [autoindex](#autoindex) / [max_client_body_size](#max_client_body_size) / [allow](#allow) / [error_page](#error_page) / [alias](#alias) / [redirect](#redirect) / [cgi_path](#cgi_path) |
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
./webserv [configuration_file] [ [-s|-l|-a|-e FLAG_ARG] ...]
```
Default configuration file: `/etc/webserv/webserv.conf`

## Flags
Flags overwrite the default settings and the ones that are configured in the config file. They can be utilized to enable options directly from the start of the program and not only after the configuration was parsed and loaded

| Flag | Description | Allowed values
| --- | --- | --- |
| s | Controls wether the server should print to stdout | on/off |
| l | Controls the debug level | 0/1/2/3 -> for error/warning/info/debug
| a | Sets the access log path | -
| e | Sets the error log file path | -

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
**Allowed tokens:** [types](#types) / [include](#include) / [log_to_stdout](#log_to_stdout) / [log_level](#log_level) / [access_log](#access_log) / [error_log](#error_log) / [server](#server)

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
**Allowed tokens:** [listen](#listen) / [server_name](#server_name) / [root](#root) / [index](#index) / [autoindex](#autoindex) / [max_client_body_size](#max_client_body_size) / [allow](#allow) / [error_page](#error_page) / [location](#location)

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
**Allowed tokens:** [alias](#alias) / [root](#root) / [index](#index) / [allow](#allow) / [autoindex](#autoindex) / [redirect](#redirect) / [max_client_body_size](#max_client_body_size) / [cgi](#cgi)

### Cgi
```nginx
cgi EXTENSION {
    cgi_path PATH;
}
```
Cgi context for `EXTENSION`. It contains the path of the cgi file.  
**Allowed tokens:** [cgi_path](#cgi_path)

## Directives

### type
```nginx
type MIME_TYPE EXTENSION [EXTENSION ...];
```
Set the mime type for the given extensions.  
**Allowed contexts:** [Types](#types)

### include
```nginx
include PATH;
```
Include another configuration file.  
**Allowed contexts:** [Http](#http) / [Server](#server) / [Location](#location) / [Cgi](#cgi)

### log_to_stdout
```nginx
log_to_stdout [on|off];
```
Sets whether logs should also be displayed on the standard output.  
Default: `off`  
**Allowed contexts:** [Http](#http)

### log_level
```nginx
log_level LEVEL;
```
Set the log level. The log level can be `debug`, `info`, `warning` or `error`.  
Default: `info`  
**Allowed contexts:** [Http](#http)

### access_log
```nginx
access_log PATH;
```
Set the path of the access log file.  
Default: `/var/log/webserv/access.log`  
**Allowed contexts:** [Http](#http)

### error_log
```nginx
error_log PATH;
```
Set the path of the error log file.  
Default: `/var/log/webserv/error.log`  
**Allowed contexts:** [Http](#http)

### listen
```nginx
listen HOST:PORT;
```
Set the host and port of the server.  
**Allowed contexts:** [Server](#server)

### server_name
```nginx
server_name NAME [NAME ...];
```
Set the server names.  
**Allowed contexts:** [Server](#server)

### root
```nginx
root PATH;
```
Set the root path of the server.  
**Allowed contexts:** [Server](#server) / [Location](#location)

### index
```nginx
index FILE [FILE ...];
```
Set the index files.  
**Allowed contexts:** [Server](#server) / [Location](#location)

### autoindex
```nginx
autoindex [on|off];
```
Enable or disable the directory listing.  
Default: `off`  
**Allowed contexts:** [Server](#server) / [Location](#location)

### max_client_body_size
```nginx
max_client_body_size SIZE;
```
Set the maximum size of the body of a request.  
Default: `1048576`  
**Allowed contexts:** [Server](#server) / [Location](#location)

### allow
```nginx
allow METHOD [METHOD ...];
```
Set the allowed methods.  
Default: `GET` / `HEAD` / `OPTIONS`  
**Allowed contexts:** [Server](#server) / [Location](#location)

### error_page
```nginx
error_page CODE PATH;
```
Set a custom error page for the given status code.  
**Allowed contexts:** [Server](#server)

### alias
```nginx
alias PATH;
```
Set an alias path. Example request: `GET /alias/file` -> `root/PATH/file`  
**Allowed contexts:** [Location](#location)

### redirect
```nginx
redirect URL;
```
Redirects the request of the location to the given url.   
**Allowed contexts:** [Server](#server) / [Location](#location)

### cgi_path
```nginx
cgi_path PATH;
```
Set the path of the cgi executable.  
**Allowed contexts:** [Cgi](#cgi)

## Example

```nginx
# File -> /example/mime.types
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
# File -> /example/webserv.conf
http {
  include /example/mime.types;

  log_to_stdout on;
  log_level info;
  access_log /var/log/webserv/access.log;
  error_log /var/log/webserv/error.log;

  include /example/server.conf;
}
```
```nginx
# File -> /example/server.conf
server {
  listen localhost:8080;
  server_name localhost:8080;
  root /example/www;
  index index.html index.htm;
  autoindex on;
  max_client_body_size 1048576;
  allow GET HEAD OPTIONS;

  error_page 404 /404.html;

  location /example {
    alias /example/www; # Request: GET /example/file -> root/www/file
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
