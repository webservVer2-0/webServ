#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

/**
 * @file webserv.hpp
 * @brief Web server with compatiblity with HTTP/1.1.
 * @version 0.1
 * @date 2023-02-08
 *
 * @copyright Copyright (c) 2023
 *
 */

/********************************************/
// C headers
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/event.h>
// #include <sys/socket.h>
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
#include <sstream>
#include <string>
#include <vector>

/********************************************/
// self headers
#include "cache.hpp"
#include "color.hpp"
#include "config.hpp"
#include "datas.hpp"
#include "errorchecker.hpp"
#include "mime.hpp"
#include "server.hpp"
#include "utils.hpp"

#define SOUT std::cout
#define SEND '\n'

#define WEBSERV "webserv"
#define CANNOTFOUND "Can not find Config file"
#define CRITICAL "Issue a critical problem"

#endif
