#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

ssize_t GetFileSize(const char* file_path) {
  struct stat file_info;
  int ret;

  ret = stat(file_path, &file_info);
  if (ret < 0) {
    return (-1);
  }
  return (file_info.st_size);
}

ssize_t RobustRead(int fd, void* buf, size_t nbyte) {
  ssize_t dep_size;
  ssize_t tmp_size;
  size_t buffer_size;
  char inner_buff[1024];
  char* ret_buf;
  char* dummy_buf;

  buffer_size = 1024;
  ret_buf = new char[buffer_size];
  if (ret_buf == NULL) return (-1);

  memset(ret_buf, 0, buffer_size);

  tmp_size = read(fd, inner_buff, 1024);
  if (dep_size == -1) {
    delete[] ret_buf;
    return (-1);
  }
  dep_size = 0;
  int d = 0;
  while (tmp_size != 0 || dep_size != nbyte) {
    // printf("%d\n", d++);
    if (dep_size <= buffer_size) {
      //   printf("%d\n", d++);
      memmove(ret_buf + dep_size, inner_buff, tmp_size);
      memset(inner_buff, 0, 1024);
      dep_size += tmp_size;
      //   printf("%d\n", d++);
    } else if (dep_size + tmp_size > buffer_size) {
      buffer_size += buffer_size;
      dummy_buf = new char[buffer_size];
      if (dummy_buf == NULL) return (-1);
      memmove(dummy_buf, ret_buf, dep_size);
      memmove(dummy_buf + dep_size, inner_buff, tmp_size);
      memset(inner_buff, 0, 1024);
      dep_size += tmp_size;
      delete[] ret_buf;
      ret_buf = dummy_buf;
      dummy_buf = NULL;
    }

    tmp_size = read(fd, inner_buff, 1024);
    if (tmp_size == 0) {
      tmp_size = read(fd, inner_buff, 1024);
      if (tmp_size != 0)
        continue;
      else
        break;
    } else if (tmp_size < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      continue;
    } else if (tmp_size < 0) {
      buf = NULL;
      delete[] ret_buf;
      return (-1);
    }
  }
  //   printf("%d\n", d++);
  buf = ret_buf;
  return dep_size;
}

// ssize_t RobustWrite(int fd, void* buf, size_t nbyte) {}

int main(int ac, char* argv[]) {
  int fd;
  char buf[1000000];

  fd = open(argv[1], O_RDONLY | O_NONBLOCK);

  //   ssize_t ret = RobustRead(fd, buf, GetFileSize(argv[1]));

  ssize_t ret = read(fd, buf, GetFileSize(argv[1]));
  buf[ret - 1] = '\0';

#if DG
  printf("size = %ld, ret = %ld\n", GetFileSize(argv[1]), ret);
#else
  write(1, buf, ret);
#endif
  printf("size = %ld, ret = %ld\n", GetFileSize(argv[1]), ret);
  //   delete[] buf;
  return (0);
}