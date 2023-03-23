#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "../../include/config.hpp"
#include "../../include/request_handler.hpp"
#include "../../include/utils.hpp"
#include "../../include/webserv.hpp"

void ServerInit(ServerConfig& config) {
  struct sockaddr_in* sock_ptr = config.GetServerAddress();
  int* socket_list = config.GetServerSocket();
  int server_number = config.GetServerNumber();

  for (int i = 0; i < server_number; i++) {
    socket_list[i] = socket(AF_INET, SOCK_STREAM, 0);
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

  SetSockoptReuseaddr(socket_list, server_number);

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

  //   SetSockoptReuseaddr(server_socket, server_number);

  for (int i = 0; i < server_number; i++) {
    s_server_type* udata = new s_server_type(config, i, server_socket[i]);
    ServerConfig::ChangeEvents(server_socket[i], EVFILT_READ,
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
  g_kq = kque;

  while (true) {
    new_event_number =
        kevent(kque, &config.change_list_[0], config.change_list_.size(),
               config.event_list_, max_event, nullptr);
    if (new_event_number == -1) {
      PrintError(2, WEBSERV, "kevent has error");
    }
    ServerConfig::change_list_.clear();

    for (int i = 0; i < new_event_number; i++) {
      curr_event = &config.event_list_[i];
      s_base_type* ft_filter = static_cast<s_base_type*>(curr_event->udata);
      if (ft_filter == NULL) {
        continue;
      }
      switch (ft_filter->GetType()) {
        case WORK: {
          s_work_type* work_type = static_cast<s_work_type*>(ft_filter);
          if (work_type->GetWorkType() == file) {
            std::cout << "FILE steps"
                      << " / Task FD : " << ft_filter->GetFD() << std::endl;
            if (work_type->GetClientStage() == GET_START)
              WorkGet(curr_event);
            else if (work_type->GetClientStage() == POST_START) {
              std::cout << "post start!!! " << std::endl;
              WorkFilePost(curr_event);
            }
          } else if (work_type->GetWorkType() == cgi)
            WorkCGIPost(curr_event);
          // std::cout << "cgi steps" << std::endl;
        } break;
        case CLIENT: {
          if (curr_event->filter == EVFILT_READ) {
            {
              // std::cout << "READ steps"
              //           << " / Task FD : " << ft_filter->GetFD() <<
              //           std::endl;
              if (curr_event->data == 0) {
                continue;
              } else {
                int result = 0;
                result = RequestHandler(curr_event);
                if (result == -1) continue;
              }

              switch (static_cast<s_client_type*>(ft_filter)->GetStage()) {
                case GET_READY: {
                  ClientGet(curr_event);
                  break;
                }
                case POST_READY: {
                  std::cout << "post r\n";
                  // if (static_cast<s_work_type*>(ft_filter)->GetWorkType() ==
                  //     file)
                  //     {
                  //       std::cout << "file!!!!!!!!!!!!!!!!!!\n";
                  //       ClientFilePost(curr_event);
                  //     }
                  // else
                  //   ClientCGIPost(curr_event);
                  ClientFilePost(curr_event);
                  break;
                }
                case DELETE_READY: {
                  break;
                }
                case ERR_READY: {
                  break;
                }
                default: {
                  break;
                }
              }
            }
          } else if (curr_event->filter == EVFILT_WRITE) {
            s_client_type* client = static_cast<s_client_type*>(ft_filter);
            std::cout << "WRITE steps"
                      << " / Task FD : " << ft_filter->GetFD() << std::endl;

            t_send* send = &client->GetSend();
            switch (send->flags) {
              case 0:  // Make header(header + body)
                client->SetResponse();
                send->header = MakeTopMessage(client);
              case 1:  // Make send message(no header, only body)
                send->send_msg = MakeSendMessage(client, send->header);
                SendMessageLength(client);
                SendProcess(curr_event, client);
                break;
              case 2:  // send failed(char * &sendmsg)
                SendProcess(curr_event, client);
                break;
              default:

                SendFin(curr_event, client);
            }
            /*if (client->GetStage() == RES_SEND) {
              SendProcess(curr_event, client);
            } else {
              char* msg_top = strdup("");
              char* send_msg = strdup("");
              if (client->IsChunked()) {
                send_msg = MakeSendMessage(client, msg_top);
              } else {
                client->SetResponse();
                msg_top = MaketopMessage(client);
                send_msg = MakeSendMessage(client, msg_top);
              }
              delete msg_top;
              client->SetBuf(send_msg);
              SendMessageLength(client);
              SendProcess(curr_event, client);
              if (client->GetStage() == RES_SEND) continue;
              SendFin(curr_event, client);
            }*/
          } else if (curr_event->filter == EVFILT_TIMER ||
                     curr_event->flags & EV_EOF) {
            DeleteUdata(ft_filter);
          }
        } break;
        case LOGGER: {
          if (curr_event->filter == EVFILT_WRITE) {
            std::cout << "Logger steps"
                      << " / Task FD : "
                      << static_cast<s_logger_type*>(curr_event->udata)->GetFD()
                      << std::endl;
            static_cast<s_logger_type*>(ft_filter)->PushData();
          }
        } break;
        default: {  // Server case
          sockaddr_in* addr_info = config.GetServerAddress();
          std::cout << "SERVER steps"
                    << " / Task FD : " << ft_filter->GetFD() << std::endl;
          socklen_t addrlen = sizeof(addr_info);
          int client_fd(accept(curr_event->ident,
                               reinterpret_cast<sockaddr*>(addr_info),
                               &addrlen));
          if (client_fd == -1) {
            PrintError(3, WEBSERV, CRITICAL, strerror(errno));
          }
          char ip_str[INET_ADDRSTRLEN];
          inet_ntop(AF_INET, &(addr_info->sin_addr), ip_str, INET_ADDRSTRLEN);
          s_server_type* server = static_cast<s_server_type*>(ft_filter);
          s_client_type* client =
              static_cast<s_client_type*>(server->CreateClient(client_fd));
          client->SetIP(ip_str);
          ServerConfig::ChangeEvents(client_fd, EVFILT_READ, EV_ADD, 0, 0,
                                     client);

          ServerConfig::ChangeEvents(client_fd, EVFILT_WRITE,
                                     EV_ADD | EV_DISABLE, 0, 0, client);
          int timer = atoi(client->GetConfig()
                               .main_config_.at(TIMEOUT)
                               .c_str());  // refactoring
          if (timer != 0) {
            ServerConfig::ChangeEvents(client_fd, EVFILT_TIMER, EV_ADD,
                                       NOTE_SECONDS, timer, client);
          }
        };
      }
      CheckError(curr_event);
    }
  }
  return;
}
