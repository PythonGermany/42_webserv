#!/usr/bin/env bash

msg="test=1"

CONTENT_TYPE=application/x-www-form-urlencoded
CONTENT_LENGTH=${#msg}
REDIRECT_STATUS=true
SCRIPT_FILENAME=tmp.php
REQUEST_METHOD=POST
GATEWAY_INTERFACE=CGI/1.1
export REDIRECT_STATUS
export SCRIPT_FILENAME
export REQUEST_METHOD
export GATEWAY_INTERFACE
export CONTENT_TYPE
export CONTENT_LENGTH

echo $msg | php-cgi

