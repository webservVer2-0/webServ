#include <iostream>
#include <string>
#include <istream>
int main()
{
	// std::ifstream a("asdasd");
	// std::ifstream file("./webserv");
	std::string a("hello");

	a.append("bye");
	std::cout << a << std::endl;
	std::cout << a.size() << std::endl;
}