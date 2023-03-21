#include "get_read_test.hpp"

void for_read(struct ke* kev)
{
	s_client_type* client = static_cast<s_client_type*>(kev->udat);
	int	read_ret = read(client->fd, client->buf, BUF_SIZE);
	if (read_ret == -1 || read_ret)
	{
		for (int i = 0; i < read_ret; i++)
		{
			client->vec.push_back(client->buf[i]);
		}
		return ;
	}
	//TODO : buf content 다 0 대입
	if (client->vec.size() != client->len)
	{
		std::cout << "client->vec.size() != len\n";
		exit(1);
	}
	int	idx = 0;
	std::vector<char>::iterator it = client->vec.begin();
	for (idx = 0; (idx < BUF_SIZE) && it != client->vec.end(); idx++)
	{
		client->buf[idx] = *(it++);
	}
	client->buf[idx -1] = '\0';
	std::cout << "client->len : " << client->len << std::endl;
	std::cout << "client->buf : " << strlen(client->buf) << std::endl;
	std::cout << "buf : " << client->buf << std::endl;
	client->type = 3;
	return ;
}

int main()
{
	struct ke* kev = new (ke);
	kev->udat = new (s_client_type);
	s_client_type* client = static_cast<s_client_type*>(kev->udat);
	client->len = (size_t)11;
	client->fd = open("a.txt", O_RDONLY);
	if (client->fd == -1) std::cout << "open() error\n";
	client->type = 2;
	try
	{	client->buf = new char[BUF_SIZE];	
		if (client->buf == NULL)
		{
			std::cout << "new error\n";
			exit(1);
		}
	} // () : for value-initialize
	catch (const std::exception& e) {std::cout << "new error()";}
	
	// https://stackoverflow.com/questions/2204176/how-to-initialise-memory-with-new-operator-in-c
	kev->udat = static_cast<void*>(client);
	while (1)
	{
		if (client->type == 2)
			for_read(kev);
		else if (client->type == 3)
			break;
	}
	// a->fd = open("screenshot.png", O_RDONLY);
	// int	fd2 = open("screenshot2.png", O_CREAT | O_WRONLY | O_APPEND, 0644);
	// if (fd2 == -1) std::cout << "open2() error\n";
		// if (write(fd2, entity, read_ret) == -1) std::cout << "write() error\n";

	
	// std::cout << "buf : " << buf << std::endl;
	// TODO : buf_size < entity_len
	// for (int i = 0; i < vec.size(); i++)
	// {	entity[i] = vec.at(i);	}
	//TODO : 위험 요소 더 알아보기
	/*
	struct stat file_info;
	int ret;

	ret = stat("a.txt", &file_info);
	if (ret < 0) {
		std::cout << "stat() error\n";
		return (-1);
	}
	file_info.st_size;
	*/
}

/* 이상해이상해코드
char* buf;
try
{	buf = new char[0];	} //
catch (const std::exception& e) {std::cout << "new error()";}

int	read_ret = 0;
std::vector<char> vec; //vec : abcdefghijk 집어넣음
std::vector<char>::iterator it = vec.begin();
int	idx;
for (idx = 0; it != vec.end(); it++)
{
	buf[idx++] = *it;
}

for (idx = 0; buf[idx]; idx++)
	std::cout << buf[idx];
*/