#include "../include/webserv.hpp"

int convert_uri(std::string rq_uri, std::map<std::string, t_loc*> location_config){
  if (rq_uri.at(0) != '/')
    return BAD_REQ;
  size_t pos = rq_uri.substr(1).find("/");
  if (pos == rq_uri.npos)
    std::cout << "not found!\n";
  std::map<std::string, t_loc*>::iterator it = location_config.find(rq_uri.substr(0, pos + 2));

  std::cout << (*it).second->location_ + rq_uri.substr(pos + 1) << std::endl;
  return 0;
}

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

  // for (int i = 0; i < webserv.GetServerNumber(); i++) {
  //   webserv.PrintTServer(i);
  //   std::cout << std::endl;
  // }
  std::cout << "-------------------------\n";
  convert_uri("/static", webserv.GetServerList(0).location_configs_);
  std::cout << "-------------------------\n";


  // ServerRun(webserv);

  (void)en;

  SOUT << "Not debug mode" << SEND;
  return (0);
}
