#include "../include/utils.hpp"

int main(int ac, char **av) {
  t_http test;

  MakeDeleteHead(test);
  MakeDeleteBody(av[1], test);
  MakeDeleteFooter(test);

  std::cout << test.entity_ << std::endl;
}