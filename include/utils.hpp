#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <cstdarg>

#include "datas.hpp"
#include "webserv.hpp"

typedef unsigned long pos_t;
class s_base_type;
typedef struct s_http t_http;
class s_client_type;

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
 * @brief Get the File object
 *
 * @param file_path
 * @return char*
 */
char* GetFile(const char* file_path);

/**
 * @brief chekc white spaces
 * @param ptr
 * @return
 */
bool IsWhiteSpace(char ptr);

/**
 * @brief whitespace 전까지, key 값까지의 길이, value 의 pos 를 반환해준다.
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

/**
 * @brief kevent udata delete 를 위한 통합 툴
 *
 * @param data server, client, work data 모두 적절하게 처리 가능하도록 설계함
 */
void DeleteUdata(s_base_type* data);

/**
 * @brief socket fd 를 위한 SO_REUSEADDR 설정을 하기 위한 util
 *
 *
 * @param socket_fd
 * @param socket_length
 */
void SetSockoptReuseaddr(int* socket_fd, int socket_length);

/* debug tools */

void PrintLine(std::string& target, pos_t pos);

/* page 제작용 */
void MakeHead(t_http& response);
void MakeFooter(t_http& response);

void MakeDeletePage(s_client_type* client, t_http& response,
                    std::string directory_path);
void MakeDeleteBody(s_client_type* client, std::string directory_path,
                    t_http& response);

void MakeAutoindexPage(t_http& response, std::string directory_path);
void MakeAutoindexBody(t_http& response, std::string directory_path);

std::string MakeNameWithoutID(std::string cookie_id, std::string file_name);
bool IsFile(struct dirent* target);
bool IsDirectory(struct dirent* target);
#endif
