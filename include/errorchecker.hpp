#ifndef errorchecker_hpp
#define errorchecker_hpp

#include <sstream>

#include "cache.hpp"
#include "config.hpp"
#include "datas.hpp"
#include "webserv.hpp"

class ServerConfig;
class s_client_type;
typedef struct s_server t_server;

void CheckError(struct kevent* curr_event);
void PutErrorPage(s_client_type* client);

#endif
