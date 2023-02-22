#include "../../include/utils.hpp"

void PrintError(int size, const char* first, ...) {
  char* val = NULL;
  va_list list;

  va_start(list, first);

  SOUT << RED;
  SOUT << first << " : ";
  for (int i = 1; i < size; i++) {
    val = va_arg(list, char*);
    SOUT << val;
    if (i + 1 != size) {
      SOUT << " : ";
    }
  }

  SOUT << RESET << SEND;
  va_end(list);
  exit(1);
}

bool IsExistFile(const char* file_path) {
  int ret;

  ret = access(file_path, R_OK | F_OK);
  if (ret == -1) {
    return (false);
  }
  return (true);
}

int GetFileSize(const char* file_path) {
  struct stat file_info;
  int ret;

  ret = stat(file_path, &file_info);
  if (ret < 0) {
    return (-1);
  }
  return (file_info.st_size);
}

char* ReadASCI(const char* file_path, int fd) {
  int size = GetFileSize(file_path);
  char* storage;
  storage = new char[size + 1];
  if (storage == NULL) {
    PrintError(3, WEBSERV, CRITICAL, "HEAP ASSIGNMENT");
  }
  if (fd == -1) {
    fd = open(file_path, O_RDONLY);
  }
  int ret = read(fd, storage, size + 1);
  if (fd == -1) {
    close(fd);
  }
  if (ret == -1) {
    PrintError(3, WEBSERV, CRITICAL, "IO READING FAIL");
  }
  storage[size] = '\0';
  return (storage);
}

bool IsWhiteSpace(char ptr) {
  return ptr == ' ' || ptr == '\t' || ptr == '\n' || ptr == '\r';
}

void PrintLine(std::string& target, pos_t pos) {
  SOUT << "Target Size : " << target.size() << SEND;
  SOUT << "Target Starting Point : " << pos << SEND;
  while (target.at(pos) != '\n') {
    SOUT << target.at(pos);
    pos++;
  }
  SOUT << SEND;
}

pos_t FindKeyLength(std::string& str, pos_t& pos) {
  pos_t ret = 0;
  while (str[pos] != ' ') {
    pos++;
    ret++;
  }
  return (ret);
}

pos_t FindValueLength(std::string& str, pos_t& pos) {
  pos_t ret = 0;
  while (IsWhiteSpace(str[pos])) {
    pos++;
  }
  pos_t i = pos;
  while (str[i] != ';') {
    if (str[i] == '\n') {
      PrintError(2, WEBSERV, "Line shoud be end with ';'");
    }
    i++;
    ret++;
  }
  return (ret);
}
