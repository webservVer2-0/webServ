#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cstdarg>

#include "webserv.hpp"

void PrintError(int size, const char* first, ...);
bool IsExistFile(const char* file_path);
int GetFileSize(const char* file_path);
char* ReadASCI(const char* file_path, int fd);
bool IsWhiteSpace(char ptr);

#endif
