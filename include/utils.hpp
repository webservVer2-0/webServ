#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cstdarg>

#include "webserv.hpp"

typedef unsigned long pos_t;

/**
 * @brief Error handling function with variable parameters.
 *
 *
 * @param size 문자열 개수
 * @param first 넣을 Error 문구(가변인자)
 * @param
 */
void PrintError(int size, const char* first, ...);

/**
 * @brief 파일 존재 여부를 확인하는 함수
 *
 *
 * @param file_path
 * @return 파일 존재하면 true, 존재하지 않으면 false
 */
bool IsExistFile(const char* file_path);

/**
 * @brief file size만을 알아내는 용도.
 *
 *
 * @param file_path
 * @return file의 사이즈 값을 정수로 반환한다. 실패시 -1 반환
 */
int GetFileSize(const char* file_path);

/**
 * @brief 다목적 Read Wrapping Function
 *
 *
 * @param file_path
 * @param storage
 * @param fd 1 이 들어오는 경우 파일을 스스로 open 하고 close 해준다.
 * @return 읽어서 얻어낸 char 문자열
 */
char* ReadASCI(const char* file_path, int fd);

/**
 * @brief chekc white spaces
 * @param ptr
 * @return
 */
bool IsWhiteSpace(char ptr);

/**
 * @brief 개행 전까지, key 값까지의 길이, value 의 pos 를 반환해준다.
 *
 * @param str
 * @param pos
 * @return int
 */
pos_t FindKeyLength(std::string& str, pos_t& pos);

/**
 * @brief 개행 전까지, pos 를 기준으로 개행 전까지 길이를 반환해준다.
 *
 *
 * @param str
 * @param pos
 * @return
 */
pos_t FindValueLength(std::string& str, pos_t& pos);

/* debug tools */

void PrintLine(std::string& target, pos_t pos);

#endif
