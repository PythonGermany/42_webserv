import sys
import argparse
import requests

def split_string_into_chunks(input_string, chunk_size):
    return [input_string[i:i + chunk_size] for i in range(0, len(input_string), chunk_size)]

def send_chunked_request(url, data):
    # Create an empty list to store the data chunks
    data_chunks = split_string_into_chunks(data, 1000)

    # Create a session to send the request
    with requests.Session() as session:
        def chunked_data():
            for chunk in data_chunks:
                yield chunk.encode()  # Encode each chunk as bytes

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

if __name__ == '__main__':
    # Define the URL of the server you want to test
    host = 'http://localhost:8080'

    # Create an argument parser to accept the request URI and data as arguments
    parser = argparse.ArgumentParser(description='Send data in chunks via PUT request')
    parser.add_argument('uri', help='Request URI')
    parser.add_argument('data', help='Data to send')
    args = parser.parse_args()

    url = host + args.uri

    # Call the function to send the chunked request
    send_chunked_request(url, args.data)
