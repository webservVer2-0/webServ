#include <stdio.h>
#include <string.h>

#include <iostream>

int main() {
  char* str1 = "hello\n\nworld!";
  char* str2 = "hello\n";
  char* str3 = "";
  char* str4 = NULL;

  printf("%d\n", memcmp(str1, str2, strlen(str1)));
  printf("%d\n", memcmp(str2, "hello\n", strlen(str2)));
  printf("%d\n", memcmp(str3, "", strlen(str3)));
  return 0;
}