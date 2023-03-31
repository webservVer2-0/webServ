#include "../../include/mime.hpp"

#include "../../include/config.hpp"

void SetMime(t_mime& storage, std::string filepath) {
  filepath.insert(0, "./");
  int extension[2];
  int mime[2];
  int file_size = GetFileSize(filepath.c_str());
  int fd = open(filepath.c_str(), O_RDONLY);

  char* raw = new char[file_size];
  if (raw == NULL) {
    PrintError(3, WEBSERV, CRITICAL, "HEAP ASSIGNMENT");
  }
  int rc = read(fd, raw, file_size);
  if (rc < 0) {
    PrintError(3, WEBSERV, CRITICAL, "READ ERROR");
  }
  raw[file_size - 1] = '\0';

  extension[0] = 0;
  extension[1] = 0;
  mime[0] = 0;
  mime[1] = 0;

  std::string temp(raw);
  std::string tmp_mime;
  std::string tmp_extension;

  int i = file_size;
  while (--i > -1) {
    if (raw[i] == '}') {
      continue;
    } else if (IsWhiteSpace(raw[i])) {
      continue;
    } else if (raw[i] == ';') {
      extension[1] = --i;
      while (!IsWhiteSpace(raw[i])) {
        i--;
      }
      extension[0] = i + 1;
      while (IsWhiteSpace(raw[i])) {
        i--;
      }
      mime[1] = i;
      while (!IsWhiteSpace(raw[i])) {
        i--;
      }
      mime[0] = i + 1;
      while (IsWhiteSpace(raw[i])) {
        i--;
        if (raw[i] == '\n') {
          break;
        }
      }
      tmp_extension =
          temp.substr(extension[0], extension[1] - extension[0] + 1);
      tmp_mime = temp.substr(mime[0], mime[1] - mime[0] + 1);
      storage.insert(pair_mime(tmp_extension, tmp_mime));
      tmp_mime.clear();
      tmp_extension.clear();
    }
  }
  close(fd);
  delete[] raw;
}
