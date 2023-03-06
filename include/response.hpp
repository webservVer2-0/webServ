#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstdarg>

#include "webserv.hpp"

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
 * @return t_html
 */
t_html MakeResponseMessages(s_client_type* client);

/**
 * @brief send 할 messages maker
 *
 * @param client
 * @return char*
 */
char* MakeSendMessage(s_client_type* client);

#endif
