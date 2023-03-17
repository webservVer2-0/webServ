#ifndef REQUEST_HANDLER_HPP_
#define REQUEST_HANDLER_HPP_

#include "webserv.hpp"

t_error request_handler(void* udata, char* msg);
t_error convert_uri(std::string rq_uri,
                    const std::map<std::string, t_loc*>& location_config,
                    s_client_type& client);

#endif