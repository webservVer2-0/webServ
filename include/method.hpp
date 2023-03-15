#ifndef METHOD_HPP_
#define METHOD_HPP_

#include <cstdarg>

#include "datas.hpp"
#include "webserv.hpp"

/**
 * @brief Request handling 종료 후 실행되는 GET method 입니다.
 * Request가 끝나고서 서버가 method 요청 처리를 시작하는 지점입니다.
 *
 * @param event 현재 처리하는 kevent 구조체
 */
void ClientGet(struct kevent* event);

/**
 * @brief case Work일때 실행되는 GET method 입니다.
 * case Client에서 추가된 method 이벤트를 처리합니다.
 *
 * @param event 현재 처리하는 kevent 구조체
 */
void WorkGet(struct kevent* event);

// POST (wip)
void ClientFilePost(struct kevent* event);
void ClientCGIPost(struct kevent* event);
void WorkFilePost(struct kevent* event);
void WorkCGIPost(struct kevent* event);

#endif
