#include "../include/webserv.hpp"

int main(int ac, char** av, char** en) {
  if (ac != 2) {
    SOUT << "Usage : ./webserv {config path}" << SEND;
    return (-1);
  }

  ServerConfig webserv(av[1]);

#if DG
  webserv.PrintServerConfig();
#else
  ;
#endif

  ServerInit(webserv);
  ServerBind(webserv);
  ServerListen(webserv);
  ServerKinit(webserv);
  ServerRun(webserv);

  (void)en;
  return (0);
}
