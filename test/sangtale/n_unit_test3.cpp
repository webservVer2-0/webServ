// #include "../../include/datas.hpp"
#include "../../include/webserv.hpp"
#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF_LEN 2
#define DOUBLE_CRLF_LEN 4
#define SOME_QUOTE "HELLOWORLD"

void test_func() {
  std::string line = "12345HelloWorld423o4k23o4";
  std::vector<char> msg;
  for (int i = 0; i < line.size(); ++i) {
    msg.push_back(line[i]);
  }
  std::vector<char> crlf;
  std::string crlf_line = "Hello";
  for (int i = 0; i < crlf_line.size(); ++i) {
    crlf.push_back(crlf_line[i]);
  }
  std::vector<char>::iterator it =
      std::search(msg.begin(), msg.end(), crlf.begin(), crlf.end());
  if (it != msg.end()) {
    std::string zz(msg.begin(), it + 10);
    std::cout << zz << "\n";
    std::cout << "begin() - it : " << std::distance(msg.begin(), it) << "\n";
  } else
    std::cout << "NOT FIND\n";
}

inline std::vector<char> crlf_make(void) {
  std::vector<char> ret;
  for (int i = 0; i < 10; ++i) {
    ret.push_back(SOME_QUOTE[i]);
  }
  return (ret);
}

void test_func2() {
  std::vector<char> r = crlf_make();
  for (int i = 0; i < r.size(); ++i) {
    std::cout << r[i] << " ";
  }
}

int main() {
  test_func();
  return 0;
}