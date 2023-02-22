/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: haryu <haryu@student.42seoul.kr>           +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/02/08 20:50:04 by haryu             #+#    #+#             */
/*   Updated: 2023/02/09 22:48:02 by haryu            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

/**
 * @file webserv.hpp
 * @author haryu
 * @brief
 * @version 0.1
 * @date 2023-02-08
 *
 * @copyright Copyright (c) 2023
 *
 */

/********************************************/
// C headers
#include <fcntl.h>
#include <sys/event.h>
#include <sys/stat.h>
#include <unistd.h>

/********************************************/
// C++ headers
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

/********************************************/
// serf headers
#include "color.hpp"
#include "config.hpp"
#include "utils.hpp"

#define SOUT std::cout
#define SEND '\n'

#define WEBSERV "webserv"
#define CANNOTFOUND "Can not find Config file"
#define CRITICAL "Issue a critical problem"

#endif
