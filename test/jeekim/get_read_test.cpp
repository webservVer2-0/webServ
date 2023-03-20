#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>

// #define BUF_SIZE 1024
#define BUF_SIZE 3

int main()
{
	struct stat file_info;
	int ret;

	ret = stat("screenshot.png", &file_info);
	if (ret < 0) {
		std::cout << "stat() error\n";
		return (-1);
	}
	file_info.st_size;
	int	fd = open("screenshot.png", O_RDONLY);
	if (fd == -1) std::cout << "open() error\n";
	int	fd2 = open("screenshot2.png", O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd2 == -1) std::cout << "open2() error\n";
	std::vector<char> vec(BUF_SIZE);
	char* buf;
	try
	{	buf = new char[BUF_SIZE];	}
	catch (const std::exception& e) {std::cout << "new error()";}

	int	read_ret = 0;
	while (read_ret = read(fd, buf, BUF_SIZE) && (read_ret != -1))//! : while(read()) > 여기서 막히기 때문에 안됨.
	{
		char* entity;
		try
		{	entity = new char[BUF_SIZE];	}
		catch (const std::exception& e) {std::cout << "new2 error()";}
		for (int i = 0; i < read_ret; i++)//read_ret == -1 이면 위험
		{
			vec.push_back(buf[i]);
			entity[i] = vec.at(i);
		}
		vec.clear();
		if (write(fd2, entity, read_ret) == -1) std::cout << "write() error\n";
		delete[] entity;
	}
	if (read_ret == -1)	std::cout << "read() error\n";
	// TODO : buf_size < entity_len
	// for (int i = 0; i < vec.size(); i++)
	// {	entity[i] = vec.at(i);	}
	// if (write(fd2, entity, BUF_SIZE) == -1) std::cout << "write() error\n";
//TODO : 위험 요소 더 알아보기
}