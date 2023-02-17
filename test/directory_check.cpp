#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  struct stat s;
  if (argc != 2) {
    printf("Usage: %s <directory>\n", argv[0]);
    return 1;
  }
  if (stat(argv[1], &s) == 0) {
    if (s.st_mode & S_IFDIR) {
      printf("%s is a directory.\n", argv[1]);
    } else {
      printf("%s is not a directory.\n", argv[1]);
    }
  } else {
    printf("Cannot access %s.\n", argv[1]);
  }
  return 0;
}
