#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>

#define BUF_SIZE 3

int main()
{
	int	fd = open("a.txt", O_RDONLY);
	if (fd == -1) std::cout << "open() error\n";

	char* buf;
	try
	{	buf = new char[BUF_SIZE];	}
	catch (const std::exception& e)
	{	std::cout << "new error()";	}

	int	read_ret = 0;
	while (read_ret = read(fd, buf, BUF_SIZE) && (read_ret != -1))//! : while(read()) > 여기서 막히기 때문에 안됨.
	{
		std::cout << buf << std::endl;
	}
	if (read_ret == -1)	std::cout << "read() error\n";
	std::cout << "buf : " << buf << std::endl;
}