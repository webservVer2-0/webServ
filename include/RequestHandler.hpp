#ifndef REQUEST_HANDLER_HPP_
#define REQUEST_HANDLER_HPP_

#include "config.hpp"
#include "datas.hpp"
#include "webserv.hpp"

int RequestHandler(struct kevent* curr_event);
t_error convert_uri(std::string rq_uri,
                    const std::map<std::string, t_loc*>& location_config,
                    s_client_type& client);

#endif