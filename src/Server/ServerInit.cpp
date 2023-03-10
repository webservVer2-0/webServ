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
  int max_connect = 0;
  int ret;

  for (int i = 0; i < server_number; i++) {
    max_connect =
        atoi(config.GetServerList(i).main_config_.at("max_connect").c_str());
    ret = listen(socket_list[i], max_connect);
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

  SetSockoptReuseaddr(server_socket, server_number);

  for (int i = 0; i < server_number; i++) {
    s_server_type* udata = new s_server_type(config, i, server_socket[i]);
    ChangeEvents(config.change_list_, server_socket[i], EVFILT_READ,
                 EV_ADD | EV_ENABLE, 0, 0, udata);
    std::cout << "[ Server(" << GREEN << std::setw(10) << std::right
              << config.GetServerList(i).main_config_.at("server_name") << RESET
              << ") : " << std::setw(30) << std::right << "Port is activated ]"
              << std::endl;
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

  while (true) {
    new_event_number =
        kevent(kque, &(config.change_list_[0]), config.change_list_.size(),
               config.event_list_, max_event, nullptr);
    if (new_event_number == -1) {
      PrintError(2, WEBSERV, "kevent has error");
    }

    config.change_list_.clear();

    for (int i = 0; i < new_event_number; i++) {
      curr_event = &config.event_list_[i];
      if (curr_event->flags & EV_ERROR) {
        PrintError(3, WEBSERV, CRITICAL, "kevent running error");
      } else {
        s_base_type* ft_filter = static_cast<s_base_type*>(curr_event->udata);
        switch (ft_filter->GetType()) {
          case WORK:
            std::cout << "work type" << std::endl;
            {
              s_work_type* work_type = static_cast<s_work_type*>(ft_filter);
              if (work_type->GetWorkType() == file)
                std::cout << "file steps" << std::endl;
              else if (work_type->GetWorkType() == cgi)
                std::cout << "cgi steps" << std::endl;
            }
            break;
          case CLIENT:
            std::cout << "client type" << std::endl;
            std::cout << "Client Id : "
                      << static_cast<s_client_type*>(ft_filter)->GetCookieId()
                      << std::endl;
            {
              if (curr_event->filter == EVFILT_READ) {
                std::cout << "client Read step" << std::endl;
                char* client_msg = new char[curr_event->data];
                int ret =
                    recv(curr_event->ident, client_msg, curr_event->data, 0);
                if (ret == -1) {
                  // TODO : error handling
                }
                std::cout << "[ client (" << curr_event->ident << ") ]"
                          << std::endl;
                write(1, client_msg, curr_event->data);
                close(curr_event->ident);
                ChangeEvents(config.change_list_, curr_event->ident, 0,
                             EV_DELETE, 0, 0, NULL);
                // DeleteUdata(static_cast<s_base_type*>(curr_event->udata));
                config.change_list_.clear();
                delete[] client_msg;
                // TODO: ?????? END??? ??????????????? ??? ????????? ????????? ??????.
                // TODO: client udata ~ file udata ?????? ?????????????????? delete ???
                // ???????????? ??????
              } else if (curr_event->filter == EVFILT_WRITE) {
                s_client_type* client = static_cast<s_client_type*>(ft_filter);
                std::cout << " client Write step" << std::endl;
                client->SetResponse();
                char* msg_top = MaketopMessage(client);
                char* send_msg = MakeSendMessage(client, msg_top);
                size_t send_msg_len;
                delete msg_top;
                if (client->GetStage() == RES_CHUNK) {
                  send_msg_len =
                      static_cast<size_t>(client->GetConfig()
                                              .main_config_.find(BODY)
                                              ->second.size());
                } else if (client->GetStage() == RES_CHUNK &&
                           client->GetChunkSize() >
                               client->GetResponse().entity_length_) {
                  send_msg_len = client->GetChunkSize() %
                                 client->GetResponse().entity_length_;
                } else if (client->GetStage() == RES_FIN) {
                  send_msg_len = 5;
                } else
                  send_msg_len = client->GetMessageLength();
                send(curr_event->ident, send_msg, send_msg_len, 0);
                DeleteSendMessage(send_msg, send_msg_len);
                if (!client->GetChunked() || client->GetStage() != RES_CHUNK) {
                  DeleteUdata(ft_filter);
                  if (client->GetStage() == END) {
                    ChangeEvents(config.change_list_, curr_event->ident,
                                 EVFILT_WRITE, EV_DELETE, 0, 0, 0);
                  } else {
                    ChangeEvents(config.change_list_, curr_event->ident,
                                 EVFILT_WRITE, EV_DISABLE, 0, 0, 0);
                    ChangeEvents(config.change_list_, curr_event->ident,
                                 EVFILT_TIMER, EV_ADD, NOTE_SECONDS, 5, 0);
                    client->SetStage(RES_FIN);
                  }
                }
              } else if (curr_event->filter == EVFILT_TIMER) {
                // TODO: time out ??????, ????????? closing ??????
              }
              // eof ??? ??????????????? ???????????? ???????????? ?????????
              //   else if (curr_event->flags == EV_EOF) {
              //     // ?????? ??????
              //     std::cout << "EOF Error, ID ??????" << std::endl;
              //     delete ft_filter;
              //     ChangeEvents(config.change_list_, curr_event->ident,
              //                  EVFILT_READ, EV_DELETE, 0, 0, NULL);
              //   }
            }
            break;
          default:  // Server case
            std::cout << "server type" << std::endl;
            {
              int client_fd(accept(curr_event->ident, NULL, NULL));
              if (client_fd == -1) { /* error handling*/
              }
              s_server_type* server = static_cast<s_server_type*>(ft_filter);
              s_client_type* client =
                  static_cast<s_client_type*>(server->CreateClient(client_fd));
              ChangeEvents(config.change_list_, client_fd, EVFILT_READ,
                           EV_ADD | EV_EOF, 0, 0, client);
              ChangeEvents(config.change_list_, client_fd, EVFILT_WRITE,
                           EV_ADD | EV_DISABLE, 0, 0, client);
              int timer =
                  atoi(client->GetConfig().main_config_.at("timeout").c_str());
              ChangeEvents(config.change_list_, client_fd, EVFILT_TIMER, EV_ADD,
                           NOTE_SECONDS, timer, client);
              //   std::cout << "time setting" << timer << std::endl;
              // TODO: socket option setting;
            }
            break;
        }
      }
      CheckError(&config, curr_event);
    }
  }
  return;
}
