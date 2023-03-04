#include "../include/webserv.hpp"

/*
    TODO : uri까지 만들어주기
*/

/* s_client_type::s_client_type(t_server* config, int client_fd,
  s_server_type* mother); */

#include <iostream>
#include <sstream>
#include <vector>

char buf[1024];

// This function validates an HTTP request message and returns true if it's
// valid, false otherwise
bool validateHttpRequest(std::string request) {
  // Split the request message into lines
  std::map<std::string, std::string> lines;
  std::stringstream requestStream(request);
  std::string line;
  while (std::getline(requestStream, line)) {
    lines.push_back(line);
  }

  // Check if the first line is valid
  if (lines.empty()) {
    return false;
  }
  std::string firstLine = lines[0];
  if (firstLine.substr(0, 3) != "GET" && firstLine.substr(0, 4) != "POST") {
    return false;
  }
  if (firstLine.find("HTTP/1.1") == std::string::npos) {
    return false;
  }

  // Check if the Host header is present
  bool hostHeaderFound = false;
  for (const auto& line : lines) {
    if (line.find("Host:") != std::string::npos) {
      hostHeaderFound = true;
      break;
    }
  }
  if (!hostHeaderFound) {
    return false;
  }

  // The request is valid if all checks passed
  return true;
}

int main() {
  std::string httpRequest1 =
      "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nConnection: "
      "close\r\n\r\n";
  std::string httpRequest2 =
      "POST /submit-form HTTP/1.1\r\nHost: www.example.com\r\nContent-Type: "
      "application/json\r\nContent-Length: "
      "42\r\n\r\n{\"name\":\"John\",\"age\":30,\"city\":\"New York\"}";

  std::cout << "Request 1 is "
            << (validateHttpRequest(httpRequest1) ? "valid" : "invalid")
            << std::endl;
  std::cout << "Request 2 is "
            << (validateHttpRequest(httpRequest2) ? "valid" : "invalid")
            << std::endl;

  return 0;
}

void init_line_parsing(s_base_type* ft_filter, std::stringstream& client_msg) {
  std::string line;

  std::getline(client_msg, line);
  if (line.empty()) {
    ft_filter->stage_
  }
}

void request_msg(s_base_type* ft_filter, char* client_msg) {
  std::stringstream request(client_msg);

  init_line_parsing(ft_filter, request);
}

void perror_exit(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

char* preprocessing(char* argv) {
  int fd = open(argv, O_RDONLY);
  if (fd <= 0) perror_exit("open()");
  int rb = read(fd, buf, sizeof(buf));
  if (rb <= 0) perror_exit("read()");
  return (buf);
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "USAGE\n$> ./unit_test http_msg\n";
    return (EXIT_FAILURE);
  }
  preprocessing(argv[1]);
  request_msg(buf);
  return (EXIT_SUCCESS);
}
