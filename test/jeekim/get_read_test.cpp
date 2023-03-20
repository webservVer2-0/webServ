#include <fcntl.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <vector>

#define BUF_SIZE 1024

int main()
{
	int	fd = open("screenshot.png", O_RDONLY);
	if (fd == -1) std::cout << "open() error\n";
	int	fd2 = open("screenshot2.png", O_CREAT | O_WRONLY, 0644);
	if (fd2 == -1) std::cout << "open2() error\n";
	std::vector<char> vec(BUF_SIZE);
	char* buf;
	try
	{	buf = new char[BUF_SIZE];	}
	catch (const std::exception& e) {std::cout << "new error()";}
	char* entity;
	try
	{	entity = new char[BUF_SIZE];	}
	catch (const std::exception& e) {std::cout << "new2 error()";}
	int	read_ret = 0;
	while (read_ret = read(fd, buf, BUF_SIZE))//! : while(read()) > 여기서 막히기 때문에 안됨.
	{
		write(fd2, buf, BUF_SIZE);
		// for (int i = 0; i < read_ret; i++)//read_ret == -1 이면 위험
		// 	vec.push_back(buf[i]);
	}
	// for (int i = 0; i < vec.size(); i++)
	// {
	// 	entity[i] = vec.at(i);
	// }
	// if (write(fd2, entity, BUF_SIZE) == -1) std::cout << "write() error\n";
//TODO : 위험 요소 더 알아보기
}