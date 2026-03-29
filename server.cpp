#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

// Simple cache
unordered_map<string, string> cache;

// Curl callback
size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  string *response = (string *)userp;
  response->append((char *)contents, size * nmemb);
  return size * nmemb;
}

string fetch_from_origin(string url) {
  CURL *curl = curl_easy_init();
  string response;

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // 👇 ADD THIS (important)
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);

    // 👇 ADD THIS DEBUG
    if (res != CURLE_OK) {
      cerr << "CURL ERROR: " << curl_easy_strerror(res) << endl;
    } else {
      cout << "Fetched " << response.size() << " bytes from origin\n";
    }

    curl_easy_cleanup(curl);
  }

  return response;
}

int main() {
  int PORT = 4000;
  string origin = "http://dummyjson.com";

  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  // Bind
  address.sin_family = AF_INET;
  address.sin_port = htons(PORT);
  address.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    return 1;
  }

  // Listen
  listen(server_fd, 3);

  cout << "Server running on port " << PORT << endl;

  while (true) {
    // Accept connection
    new_socket =
        accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

    // Read request
    char buffer[5000] = {0};
    read(new_socket, buffer, 5000);

    string request(buffer);

    // Extract first line
    size_t pos = request.find("\r\n");
    string first_line = request.substr(0, pos);

    // Parse method and path
    size_t m_end = first_line.find(" ");
    size_t p_end = first_line.find(" ", m_end + 1);

    string method = first_line.substr(0, m_end);
    string path = first_line.substr(m_end + 1, p_end - m_end - 1);

    cout << "Request: " << method << " " << path << endl;

    // Cache key
    string key = method + ":" + path;

    string body;
    string status;

    // Check cache
    if (cache.find(key) != cache.end()) {
      body = cache[key];
      status = "HIT";
      cout << "Cache HIT\n";
    } else {
      string full_url = origin + path;
      cout << "Fetching from origin: " << full_url << endl;

      body = fetch_from_origin(full_url);
      cache[key] = body;
      status = "MISS";
    }

    // Build response
    string response = "HTTP/1.1 200 OK\r\n"
                      "Content-Type: application/json\r\n"
                      "X-Cache: " +
                      status +
                      "\r\n"
                      "\r\n" +
                      body;

    // Send response
    send(new_socket, response.c_str(), response.size(), 0);

    close(new_socket);
  }

  return 0;
}