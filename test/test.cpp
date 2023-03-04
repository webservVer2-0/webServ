#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

int main(void) {
  std::map<std::string, std::string> init_line;
  std::vector<std::string> vector;
  const char* httpRequest =
      "GET /index.html HTTP/1.1\r\nHost: www.example.com\r\nConnection: "
      "close\r\n\r\n";
  std::istringstream http_msg;
  http_msg.str(httpRequest);
  std::string line;
  std::string new_line;

  for (std::string line; std::getline(http_msg, line);) {
    new_line.append(line);
  }
  std::cout << new_line;
  // std::vector<std::string>::iterator it = vector.begin();
  // for (; it != vector.end(); ++it) {
  //   std::cout << *it;
  //   std::cout << "what's wrong?";
  // }
  // size_t space = httpRequest.find(" ");
  // std::cout << httpRequest.substr(0, space);
  // std::cout << httpRequest.substr(space + 1, );
  // init_line.insert(std::make_pair("GET", ""));
  return 0;
}
