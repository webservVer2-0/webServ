#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cstdarg>

#include "webserv.hpp"

typedef unsigned long pos_t;

void PrintError(int size, const char* first, ...);
bool IsExistFile(const char* file_path);
int GetFileSize(const char* file_path);
char* ReadASCI(const char* file_path, int fd);
bool IsWhiteSpace(char ptr);
/**
 * @brief 개행 전까지, key 값까지의 길이, value 의 pos 를 반환해준다.
 *
 * @param str
 * @param pos
 * @return int
 */
pos_t FindKeyLength(std::string& str, pos_t& pos);
pos_t FindValueLength(std::string& str, pos_t& pos);

/* debug tools */

void PrintLine(std::string& target, pos_t pos);

#endif
