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
	client->buf[idx - 1] = '\0';
	for (; idx < BUF_SIZE -1; idx++)
		client->buf[idx] = '\0';

	std::cout << "buf : " << client->buf << std::endl;
	client->type = 3;
	return ;
}

int main()
{
	struct stat file_info;
	int ret;

	ret = stat("a.txt", &file_info);
	if (ret < 0)
	{
		std::cout << "stat() error\n";
		exit(-1);
	}
	struct ke* kev = new (ke);
	kev->udat = new (s_client_type);
	s_client_type* client = static_cast<s_client_type*>(kev->udat);
	client->len = file_info.st_size;
	client->fd = open("a.txt", O_RDONLY);
	if (client->fd == -1) std::cout << "open() error\n";
	client->type = 2;
	try
	{	client->buf = new char[BUF_SIZE]();	
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
		std::cout << "<3\n";
		if (client->type == 2)
			for_read(kev);
		if (client->type == 3)
			break;
	}
	//TODO : 위험 요소 더 알아보기
}