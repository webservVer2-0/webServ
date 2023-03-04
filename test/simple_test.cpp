#include "../include/webserv.hpp"

/* test 1*/

/*
t_error _() { return NO_ERROR; }
t_error __() { return NOT_IMPLE; }
int main(void) {
  std::string tokens = "HTTP/1.1";
  std::string http = tokens.substr(0, 4);
  std::string ver = tokens.substr(4, 6);
  std::cout << http << "\n";
  std::cout << ver << "\n";
  std::cout << tokens.find("/") << "\n";
  std::cout << tokens.length() << "\n";

  t_error err = NO_ERROR;
  if ((err = _())) std::cout << "no_error\n";
  if ((err = __())) std::cout << "not_imple\n";
  return 0;
}
*/

/* test 2*/
int main() {
  std::string ex = "Hello\r\n\r\nWorld!";
  std::string::size_type pos = ex.find("\r\n\r\n");
  std::string::size_type pos1 = ex.find_first_not_of("\r\n\r\n", pos);
  if (pos1 == std::string::npos)
    std::cout << "?! zz\n";
  else
    std::cout << ex[pos1];
}