#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/stat.h>

#define BUF_SIZE 3
// #define BUF_SIZE 1024

int main()
{
	struct stat file_info;
	int ret;

	ret = stat("a.txt", &file_info);
	if (ret < 0) {
		std::cout << "stat() error\n";
		return (-1);
	}
	file_info.st_size;
	int	fd = open("a.txt", O_RDONLY);
	if (fd == -1) std::cout << "open() error\n";
	int	fd2 = open("b.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
	if (fd2 == -1) std::cout << "open2() error\n";
	std::vector<char> vec;
	std::vector<char>::iterator it;
	char* buf;
	try
	{	buf = new char[BUF_SIZE]();	} // () : for value-initialize
	// https://stackoverflow.com/questions/2204176/how-to-initialise-memory-with-new-operator-in-c
	catch (const std::exception& e) {std::cout << "new error()";}
	char* entity;
	try
	{	entity = new char[BUF_SIZE]();	}
	catch (const std::exception& e) {std::cout << "new2 error()";}

	int	read_ret = 0;
	while ((read_ret = read(fd, buf, BUF_SIZE)) && (read_ret != -1))//! : while(read()) > 여기서 막히기 때문에 안됨.
	{
		for (int i = 0; i < read_ret; i++)//read_ret == -1 이면 위험
		{
			vec.push_back(buf[i]);
			// entity[i] = vec.at(i);
		}
		// if (write(fd2, entity, read_ret) == -1) std::cout << "write() error\n";
		// delete[] entity;
	}
	//TODO : buf content 다 0 대입
	if (read_ret == -1)	std::cout << "read() error\n";
	int	idx = 0;
	for (it = vec.begin(); it != vec.end(); it++)
	{
		buf[idx++] = *it;
		// std::cout << *it;
	}
	for (int i = 0; buf[i]; i++)
		std::cout << buf[i];
	std::cout << std::endl;
	// printf("%s\n", buf);
	// std::cout << "buf : " << buf << std::endl;
	// TODO : buf_size < entity_len
	// for (int i = 0; i < vec.size(); i++)
	// {	entity[i] = vec.at(i);	}
	// if (write(fd2, entity, BUF_SIZE) == -1) std::cout << "write() error\n";
//TODO : 위험 요소 더 알아보기
}