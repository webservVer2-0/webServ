#ifndef errorchecker_hpp
#define errorchecker_hpp

#include "cache.hpp"
#include "config.hpp"
#include "datas.hpp"
#include "webserv.hpp"

// TODO : cache 를 config 내부에 등록 하는 방식으로 하여 작동하도록 하자
// t_cache 구조체 활용함

class ServerConfig;
class s_client_type;
typedef struct s_server t_server;

void CheckError(ServerConfig* config, struct kevent* curr_event);
void PutErrorPage(s_client_type* client, const t_server& self_config);

#endif
