#ifndef server_hpp
#define server_hpp

#include "webserv.hpp"

class ServerConfig;

void ServerInit(ServerConfig& config);
void ServerBind(ServerConfig& config);
void ServerListen(ServerConfig& config);
void ServerKinit(ServerConfig& config);
void ServerRun(ServerConfig& config);

#endif
