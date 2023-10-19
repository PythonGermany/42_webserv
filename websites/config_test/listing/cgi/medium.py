import cgi
import os

# Function to print HTTP headers
def print_headers():
    print("Content-type: text/html\n")
    print("<html>")
    print("<head>")
    print("<title>CGI Test</title>")
    print("</head>")
    print("<body>")

# Function to print the footer and close HTML tags
def print_footer():
    print("</body>")
    print("</html>")

# Main program
print_headers()
print("<h1>Python CGI Test</h1>")

# Display server information
print("<h2>Server Information</h2>")
print("<p>Server Name: " + os.environ["SERVER_NAME"] + "</p>")
print("<p>Server Port: " + os.environ["SERVER_PORT"] + "</p>")
print("<p>Server Software: " + os.environ["SERVER_SOFTWARE"] + "</p>")

# Process form data if available
form = cgi.FieldStorage()

if "user_input" in form:
    user_input = form["user_input"].value
    print("<h2>User Input</h2>")
    print("<p>You entered: " + user_input + "</p>")
else:
    print("<h2>Input Form</h2>")
    print("<form method='post'>")
    print("  <label for='user_input'>Enter something: </label><input type='text' name='user_input'><br>")
    print("  <input type='submit' value='Submit'>")
    print("</form>")

print_footer()
