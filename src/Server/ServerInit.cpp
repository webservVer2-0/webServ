#include "../../include/webserv.hpp"

void ServerInit(ServerConfig& config) {
  struct sockaddr_in* sock_ptr = config.GetServerAddress();
  int* socket_list = config.GetServerSocket();
  int server_number = config.GetServerNumber();

  for (int i = 0; i < server_number; i++) {
    socket_list[i] = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_list[i] == -1) {
      // TODO: error handling
    }
    memset(&sock_ptr[i], 0, sizeof(sock_ptr[i]));
    sock_ptr[i].sin_family = AF_INET;
    sock_ptr[i].sin_addr.s_addr = htonl(INADDR_ANY);
    sock_ptr[i].sin_port = htons(config.GetServerPort(i));
    // std::cout << std::setw(18) << std::left << "[ ● Server port   : " <<
    // GREEN
    //           << std::setw(21) << std::right << config.GetServerPort(i) <<
    //           RESET
    //           << " is ready ]" << std::endl;
  }
  return;
}

void ServerBind(ServerConfig& config) {
  struct sockaddr_in* sock_ptr = config.GetServerAddress();
  int* socket_list = config.GetServerSocket();
  int server_number = config.GetServerNumber();
  int ret;

  for (int i = 0; i < server_number; i++) {
    ret = bind(socket_list[i], reinterpret_cast<struct sockaddr*>(&sock_ptr[i]),
               sizeof(sock_ptr));
    if (ret == -1) {
      // TODO: error handling;
    }
  }
  return;
}

void ServerListen(ServerConfig& config) {
  int* socket_list = config.GetServerSocket();
  int server_number = config.GetServerNumber();
  int ret;

  for (int i = 0; i < server_number; i++) {
    ret = listen(socket_list[i], 10);
    if (ret == -1) {
      // TODO: error handling
    }
    fcntl(socket_list[i], F_SETFL, O_NONBLOCK);
  }
  return;
}

void ServerKinit(ServerConfig& config) {
  int server_number = config.GetServerNumber();

  config.SetServerKque(kqueue());
  if (config.GetServerKque() == -1) {
    // TODO: error handling
  }
  for (int i = 0; i < server_number; i++) {
    // TODO: change Event list how
    std::cout << "[ ● Server " << GREEN << std::setw(8) << std::right
              << config.GetServerPort(i) << RESET << " : " << std::setw(30)
              << std::right << "Port is activated ]" << std::endl;
  }
  return;
}

void ServerRun(ServerConfig& config) {
  (void)config;
  return;
}
