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
std::string enumToString(int code);

/**
 * @brief responsemessage maker
 *
 * @param client
 * @return t_http
 */
t_http MakeResponseMessages(s_client_type* client);

/**
 * @brief send 할 messages maker
 *
 * @param client
 * @return char*
 */
char* MakeSendMessage(s_client_type* client);

#endif
