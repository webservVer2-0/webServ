#include <arpa/inet.h>
#include <netinet/in.h>

#include <iostream>

int main() {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);                          // 예시 포트
  inet_pton(AF_INET, "192.168.0.1", &addr.sin_addr);  // 예시 IP 주소

  char ip_str[INET_ADDRSTRLEN];
  inet_ntop(AF_INET, &(addr.sin_addr), ip_str, INET_ADDRSTRLEN);

  std::cout << "IP address: " << ip_str << std::endl;
  std::cout << "Port number: " << ntohs(addr.sin_port) << std::endl;

  return 0;
}
