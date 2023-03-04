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

  std::map<std::string, std::string> exercise;

  return (EXIT_SUCCESS);
}
