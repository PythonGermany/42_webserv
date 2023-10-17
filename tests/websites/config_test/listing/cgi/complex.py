import cgi

# Function to print HTTP headers
def print_headers():
    print("Content-type: text/html\n")
    print("<html>")
    print("<head>")
    print("<title>Advanced Python CGI Form</title>")
    print("</head>")
    print("<body>")

# Function to print the footer and close HTML tags
def print_footer():
    print("</body>")
    print("</html>")

# Main program
print_headers()
print("<h1>Advanced Python CGI Form</h1>")

# Process form data
form = cgi.FieldStorage()

if "submit" in form:
    user_input = form.getvalue("user_input")
    email = form.getvalue("email")

    # Display user input
    print("<h2>Form Submission</h2>")
    print(f"<p>You submitted: {user_input}</p>")
    print(f"<p>Email: {email}</p>")

# Display the input form
print("<h2>Contact Form</h2>")
print("<form method='post' enctype='multipart/form-data'>")
print("  <label for='user_input'>Message: </label><textarea name='user_input' rows='4' cols='50'></textarea><br>")
print("  <label for='email'>Email: </label><input type='text' name='email'><br>")
print("  <input type='submit' name='submit' value='Submit'>")
print("</form>")

print_footer()