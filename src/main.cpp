#include "../include/webserv.hpp"

int main(int ac, char **av, char **en) {
  if (ac != 2) {
    SOUT << "Usage : ./webserv {config path}" << SEND;
    return (-1);
  }

  ServerConfig webserv(av[1]);

  webserv.PrintServerConfig();

  ServerInit(webserv);
  ServerBind(webserv);
  ServerListen(webserv);
  ServerKinit(webserv);

  for (int i = 0; i < webserv.GetServerNumber(); i++) {
    webserv.PrintTServer(i);
    std::cout << std::endl;
  }

  ServerRun(webserv);

  (void)en;
#if DG
  SOUT << "debug mode" << SEND;
  system("leaks webserv");
#else
  SOUT << "Not debug mode" << SEND;
#endif
  return (0);
}
