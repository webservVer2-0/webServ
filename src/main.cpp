/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haryu <haryu@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/08 21:11:00 by haryu             #+#    #+#             */
/*   Updated: 2023/02/23 12:59:05 by haryu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

int main(int ac, char **av, char **en) {
  if (ac != 2) {
    SOUT << "Usage : ./webserv {config path}" << SEND;
    return (-1);
  }
  //   SOUT << "webserv initializing" << SEND;

  ServerConfig webserv(av[1]);

  webserv.PrintServerConfig();

  ServerInit(webserv);
  ServerBind(webserv);
  ServerListen(webserv);
  ServerKinit(webserv);
  ServerRun(webserv);

  // TODO: server init(portnumber, )
  // TODO: bind
  // TODO: listen
  // TODO: kque init
  // TODO: kevent 등록
  // TODO: 조건 1) client 연결, 2) client READ 활성화-WRITE 비활성화 3) file
  // fd READ활성화 4) client fd-WRITE 활성화

  (void)en;
#if DG
  SOUT << "debug mode" << SEND;
  system("leaks webserv");
#else
  SOUT << "Not debug mode" << SEND;
#endif
  return (0);
}
