import cgi
import os

# Function to print HTTP headers
def print_headers():
    print("Content-type: text/html\n")
    print("<html>")
    print("<head>")
    print("<title>Common Environment Variables</title>")
    print("</head>")
    print("<body>")

# Function to print the footer and close HTML tags
def print_footer():
    print("</body>")
    print("</html>")

# Main program
print_headers()
print("<h1>Common Environment Variables</h1>")

# List of common environment variables
common_env_variables = [
    "SERVER_NAME",
    "SERVER_PORT",
    "SERVER_SOFTWARE",
    "REMOTE_ADDR",
    "REMOTE_HOST",
    "HTTP_USER_AGENT",
    "HTTP_REFERER",
    "QUERY_STRING",
    "REQUEST_METHOD",
    "HTTP_ACCEPT",
    "HTTP_ACCEPT_LANGUAGE",
    "HTTP_ACCEPT_ENCODING",
    "HTTP_CONNECTION",
    "HTTP_HOST",
    "HTTP_CACHE_CONTROL",
    "HTTP_COOKIE",
    "CONTENT_LENGTH",
    "CONTENT_TYPE",
]

# Access and display common environment variables
print("<h2>Common Environment Variables</h2>")
for var_name in common_env_variables:
    var_value = os.environ.get(var_name, "Not available")
    print(f"<p>{var_name}: {var_value}</p>")

print_footer()