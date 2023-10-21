import requests
import sys
import os

# Define the target URL
target_url = "http://localhost:8080"  # Replace with the actual URL you want to send requests to

# List of requests to send with associated data (URL, expected response code, and expected content file)
requests_data = [
    {
        "url": "/",
        "expected_response": 200,
        "content_file": "",
    },
    {
        "url": "/notallowed/",
        "expected_response": 405,
        "content_file": "../websites/config_test/405.html",
    },
    {
        "url": "/notexistingpage/",
        "expected_response": 404,
        "content_file": "",
    },
    {
        "url": "/isthisworking%3FIhopeitis.html",
        "expected_response": 200,
        "content_file": "../websites/config_test/isthisworking?Ihopeitis.html",
    },
    {
        "url": "/lol😀.html",
        "expected_response": 200,
        "content_file": "",
    },
    {
        "url": "/redirect/",
        "expected_response": 200,
        "content_file": "",
    },
    {
        "url": "/redirect2/",
        "expected_response": 200,
        "content_file": "",
    },
    {
        "url": "/listing/",
        "expected_response": 200,
        "content_file": "",
    },
    {
        "url": "/index/",
        "expected_response": 200,
        "content_file": "../websites/config_test/multiindex/multiindex.html",
    },
    {
        "url": "/kapouet/pouic/toto/pouet",
        "expected_response": 200,
        "content_file": "../websites/config_test/pouic/toto/pouet",
    }
]

# Function to send requests and check responses
def send_and_check_requests():
    for request_data in requests_data:
        full_url = target_url + request_data["url"]
        try:
            response = requests.get(full_url)
            if response.status_code == request_data["expected_response"]:
                # Check if the content file exists
                content_file = request_data["content_file"]
                if content_file and not os.path.isfile(content_file):
                    print(f"\033[31mExpected content file for {request_data['url']} is not a file.\033[0m")
                    sys.exit(1)  # Exit with a non-zero status code on error

                # Read and compare response content as strings with the correct character encoding
                if content_file:
                    with open(content_file, "r", encoding="utf-8") as expected_file:
                        expected_content = expected_file.read()
                    response_content = response.text

                    if response_content.strip() != expected_content.strip():
                        print(f"\033[31mResponse content for {full_url} does not match expected content.\033[0m")
                        print("##################################################################")
                        print(response.text.strip())
                        print("##################################################################")
                        print(expected_content.strip())
                        print("##################################################################")
                        sys.exit(1)  # Exit with a non-zero status code on error

                print(f"\033[32mRequest to {full_url} succeeded with expected response {request_data['expected_response']}.\033[0m")
            else:
                print(f"\033[31mRequest to {full_url} failed. Expected {request_data['expected_response']}, got {response.status_code}.\033[0m")
                sys.exit(1)  # Exit with a non-zero status code on error
        except requests.exceptions.RequestException as e:
            print(f"\033[31mRequest to {full_url} failed: {e}\033[0m")
            sys.exit(1)  # Exit with a non-zero status code on error

if __name__ == "__main__":
    send_and_check_requests()
    sys.exit(0)  # Exit with a status code of 0 on success