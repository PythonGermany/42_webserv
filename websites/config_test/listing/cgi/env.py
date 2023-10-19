#!/usr/bin/env python
import cgi
import os

# Function to print HTTP headers
def print_headers():
    print("Content-type: text/html\n")
    print("<html>")
    print("<head>")
    print("<title>All Environment Variables</title>")
    print("</head>")
    print("<body>")

# Function to print the footer and close HTML tags
def print_footer():
    print("</body>")
    print("</html>")

# Main program
print_headers()
print("<h1>All Environment Variables</h1>")

# Iterate over all environment variables and display their values
print("<h2>All Environment Variables</h2>")
for var_name, var_value in os.environ.items():
    print(f"<p>{var_name}: {var_value}</p>")

print_footer()
