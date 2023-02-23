#include "../../include/webserv.hpp"

void ServerInit(ServerConfig& config) {
  struct sockaddr_in* sock_ptr = config.GetServerAddress();
  int* socket_list = config.GetServerSocket();
  int server_number = config.GetServerNumber();

  for (int i = 0; i < server_number; i++) {
    socket_list[i] = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_list[i] == -1) {
      PrintError(2, WEBSERV, "Server socket function is failed");
    }
    memset(&sock_ptr[i], 0, sizeof(sock_ptr[i]));
    sock_ptr[i].sin_family = AF_INET;
    sock_ptr[i].sin_addr.s_addr = htonl(INADDR_ANY);
    sock_ptr[i].sin_port = htons(config.GetServerPort(i));
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
               sizeof(sock_ptr[i]));
    if (ret == -1) {
      PrintError(2, WEBSERV, "Server socket binding function is failed");
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
      PrintError(2, WEBSERV, "Server socket listening is failed");
    }
    fcntl(socket_list[i], F_SETFL, O_NONBLOCK);
  }
  return;
}

void ServerKinit(ServerConfig& config) {
  int server_number = config.GetServerNumber();
  int* server_socket = config.GetServerSocket();

  config.SetServerKque(kqueue());
  if (config.GetServerKque() == -1) {
    PrintError(2, WEBSERV, "Server kque is malfunctioned");
  }
  for (int i = 0; i < server_number; i++) {
    // std::cout << "Socket : " << server_socket[i] << std::endl;
    ChangeEvents(config.change_list_, server_socket[i], EVFILT_READ,
                 EV_ADD | EV_ENABLE, 0, 0, NULL);
    std::cout << "[ ● Server " << GREEN << std::setw(8) << std::right
              << config.GetServerPort(i) << RESET << " : " << std::setw(30)
              << std::right << "Port is activated ]" << std::endl;
  }
  return;
}

void ServerRun(ServerConfig& config) {
  struct kevent* curr_event;
  int max_event = config.max_connection;
  int new_event_number = 0;
  int kque = config.GetServerKque();

  std::cout << "[ "
            << "WebServ Is Activated"
            << " ]" << std::endl;
  //   std::cout << config.GetServerKque() << std::endl;
  while (true) {
    new_event_number =
        kevent(kque, &(config.change_list_[0]), config.change_list_.size(),
               config.event_list_, max_event, nullptr);
    if (new_event_number == -1) {
      PrintError(2, WEBSERV, "kevent has error");
    }
    std::cout << "Event 갯수 : " << new_event_number << std::endl;

    config.change_list_.clear();

    for (int i = 0; i < new_event_number; i++) {
      curr_event = &config.event_list_[i];
      if (curr_event->flags & EV_ERROR) {
        PrintError(3, WEBSERV, CRITICAL, "kevent running error");
      } else {
        std::cout << "something happens!" << std::endl;
        // TODO : fd에 대한 udata를 활용해서 구분짓는게 가장 편리함.
        // TODO : 근데, 서버 fd, 클라이언 fd, 파일 fd, cgi fd 를 구분하기
        // 어려우므로 기반 클래스를 하나 만들고, 해당 클래스에서 확산하는
        // 방식으로 접근하는게 답으로 보임..
      }
    }
  }
  return;
}
