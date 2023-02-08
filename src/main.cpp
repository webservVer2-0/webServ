/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haryu <haryu@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/08 21:11:00 by haryu             #+#    #+#             */
/*   Updated: 2023/02/08 21:31:42 by haryu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

int main(int ac, char **av, char **en) {
  if (ac != 2) {
    SOUT << "Usage : ./webserv {config path}" << SEND;
    return (-1);
  }
  SOUT << "webserv initializing" << SEND;
  (void)av;
  (void)en;
  return (0);
}
