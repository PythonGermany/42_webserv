import cgi

form = cgi.FieldStorage()

print("Content-type: text/html\n")
print("<html>")
print("<head>")
print("<title>CGI Form Test</title>")
print("</head>")
print("<body>")
print("<h1>CGI Form Test</h1>")

if "name" in form:
    name = form["name"].value
    print(f"<p>Hello, {name}!</p>")
else:
    print("<p>No name provided.</p>")

print("<form method='post'>")
print("  <label for='name'>Name: </label><input type='text' name='name'><br>")
print("  <input type='submit' value='Submit'>")
print("</form>")

print("</body>")
print("</html>")
