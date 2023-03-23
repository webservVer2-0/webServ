#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstdarg>

#include "datas.hpp"
#include "webserv.hpp"

typedef struct s_http t_http;
/**
 * @brief enum to string
 *
 * @param code
 * @return std::string
 */
std::string EnumToString(int code);

/**
 * @brief responsemessage setting 함수
 *
 * @param client
 * @return t_http
 */
t_http MakeResponseMessages(s_client_type* client);

/**
 * @brief responsemessage의 header 부문 char*화 처리
 *
 * @param client
 * @return char*
 */
char* MakeTopMessage(s_client_type* client);

/**
 * @brief send 함수에 넣을 char* 제작
 * - 기본적으로는 MakeTopMessage에서 만들었던 header와 entity를 합치는 함수
 * - chunked의 경우 header는 이미 보냈다고 생각하고 entity_size와 entity만 만듦
 * @param client
 * @return char*
 */
char* MakeSendMessage(s_client_type* client, char* msg);

/**
 * @brief send 함수에서 사용할 send할 msg의 length 길이를 정하는 함수
 *
 * @param client
 */
void SendMessageLength(s_client_type* client);

/**
 * @brief send함수 실패시 재호출을 위한 함수
 *
 * @param event
 * @param client
 */
void SendProcess(struct kevent* event, s_client_type* client);

/**
 * @brief client_type의 write_event 마무리(이벤트 탈출 및 초기화)
 *
 * @param event
 * @param client
 */
void SendFin(struct kevent* event, s_client_type* client);

#endif
