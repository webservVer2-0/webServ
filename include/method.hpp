#ifndef METHOD_HPP_
#define METHOD_HPP_

#include <cstdarg>

#include "datas.hpp"
#include "webserv.hpp"

/**
 * @brief
 *
 * @param event
 */
void ClientGet(struct kevent* event);

/**
 * @brief
 *
 * @param event
 */
void WorkGet(struct kevent* event);

#endif
