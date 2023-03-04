#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../include/webserv.hpp"

void vector_print(std::vector<std::string>& vec, std::string vector_name) {
  std::vector<std::string>::iterator it = vec.begin();
  std::cout << "<< " << vector_name << " >>\n";
  int i = 0;
  for (; it != vec.end(); ++it) {
    std::cout << "index[" << i << "] : ";
    std::cout << *it << "\n";
    ++i;
  }
}

std::vector<std::string>::iterator vector_checker(
    std::vector<std::string>& lines, std::string to_find) {
  std::vector<std::string>::iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    if (*it == to_find) {
      return (it);
    }
  }
  return (it);
}

void test_func(std::string line) {
  // std::cout << "befor\n";
  // std::cout << line;

  // std::cout << "\n\n";

  // 헤더 파싱
  std::string::size_type pos = line.find("\r\n\r\n");
  std::string::size_type endpos = line.find("\r\n");
  std::string header_lines = line.substr(endpos + 2, pos - endpos - 2);

  // std::cout << "aft\n";
  // std::cout << header_lines;
  std::string::size_type last_pos = 0;
  std::string::size_type next_pos = 0;
  std::string::size_type colon_pos = 0;
  try {
    while (last_pos != header_lines.size()) {
      // 한 줄씩 파싱
      std::cout << "last pos : " << last_pos << "\n";
      std::cout << "next pos : " << next_pos << "\n";
      std::cout << "colon pos : " << colon_pos << "\n";
      next_pos = header_lines.find("\r\n", last_pos);
      std::cout << "-1-\n";
      if (next_pos == std::string::npos) {
        next_pos = header_lines.size();
      }
      std::cout << "-1.5-\n";
      std::string line = header_lines.substr(last_pos, next_pos - last_pos);
      std::cout << "-2-\n";
      // 콜론을 찾아서 key-value로 분리
      colon_pos = line.find(":");
      if (colon_pos != std::string::npos) {
        std::string key = line.substr(0, colon_pos);
        std::string value = line.substr(colon_pos + 2);  // 콜론과 스페이스 제외
      }
      std::cout << "-3-\n";
      last_pos = next_pos + 2;
    }
  } catch (std::out_of_range) {
    std::cout << "머선일이고\n";
  }
}

int main(void) {
  std::map<std::string, std::string> init_line;
  std::vector<std::string> tokens;
  std::vector<std::string> lines;
  const char* httpRequest =
      "GET /index.html HTTP/1.1\r\nHost: "
      "www.example.com\r\nConnection: "
      "close\r\n\r\nRequest body: hello world";
  std::stringstream http_msg(httpRequest);
  std::stringstream copy(httpRequest);
  std::string line;

  test_func(httpRequest);
  // while (http_msg >> line) {
  //   tokens.push_back(line);
  // }
  // while (std::getline(copy, line)) {
  //   lines.push_back(line);
  // }
  // vector_print(tokens, "tokens");
  // vector_print(lines, "lines");
  // std::vector<std::string>::iterator it = vector_checker(lines, "");
  // if (it != lines.end()) std::cout << "find!";
  // size_t space = httpRequest.find(" ");
  // std::cout << httpRequest.substr(0, space);
  // std::cout << httpRequest.substr(space + 1, );
  // init_line.insert(std::make_pair("GET", ""));
  return 0;
}
