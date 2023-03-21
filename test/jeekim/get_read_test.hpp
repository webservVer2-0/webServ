#ifndef GET_READ_TEST_HPP
#define GET_READ_TEST_HPP

#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>
#include <cstring>

#define BUF_SIZE 3
// #define BUF_SIZE 1024

class s_client_type
{
	public :
		int		type;
		char*	buf;
		size_t	len;
		int		fd;
		std::vector<char> vec;
};

struct ke
{
	void *udat;
};

void for_read(struct ke* kev);

#endif