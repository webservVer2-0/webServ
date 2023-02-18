/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haryu <haryu@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/08 21:11:00 by haryu             #+#    #+#             */
/*   Updated: 2023/02/18 22:27:26 by haryu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

int main(int ac, char **av, char **en) {
  if (ac != 2) {
    SOUT << "Usage : ./webserv {config path}" << SEND;
    return (-1);
  }
  SOUT << "webserv initializing" << SEND;

  ServerConfig webserv(av[1]);

  (void)en;
#if DG
  SOUT << "debug mode" << SEND;
  system("leaks webserv");
#else
  SOUT << "Not debug mode" << SEND;
#endif
  return (0);
}
