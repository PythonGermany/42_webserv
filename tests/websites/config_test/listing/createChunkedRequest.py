import sys
import argparse
import requests

# Define the URL of the server you want to test
url = 'http://localhost:8080/upload/testChunked.txt'

# Create an argument parser to accept the file path as an argument
parser = argparse.ArgumentParser(description='Send a file in chunks via PUT request')
parser.add_argument('file_path', help='Path to the file containing the data')
args = parser.parse_args()

# Create an empty list to store the data chunks
data_chunks = []

# Open the file and read it in 1000-byte chunks, appending each chunk to the list
with open(args.file_path, 'rb') as file:
    while True:
        chunk = file.read(100)
        if not chunk:
            break
        data_chunks.append(chunk)

# Create a session to send the request
with requests.Session() as session:
  def chunked_data():
        for chunk in data_chunks:
            yield chunk

  # Send the request with the 'Transfer-Encoding: chunked' header using a PUT request
  response = session.put(url, headers={'Transfer-Encoding': 'chunked'}, data=chunked_data())

  # Print the server's response for each chunk if needed
  print(response.status_code)
  print(response.text)

  result = response.status_code == 201 or response.status_code == 204
  if result:
    sys.exit(0)  # Exit with code 0 for success
  else:
    sys.exit(1)  # Exit with a non-zero code for failure