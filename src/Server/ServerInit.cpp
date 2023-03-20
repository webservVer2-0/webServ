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
    ServerConfig::ChangeEvents(server_socket[i], EVFILT_READ,
                               EV_ADD | EV_ENABLE, 0, 0, udata);
    std::cout << "[ Server(" << GREEN << std::setw(10) << std::right
              << config.GetServerList(i).main_config_.at("server_name") << RESET
              << ") : " << std::setw(30) << std::right << "Port is activated ]"
              << std::endl;
  }
  return;
}

static size_t ChunkEncoding(s_client_type* client) {
  size_t send_msg_len;
  if (client->GetStage() == RES_CHUNK) {
    send_msg_len = static_cast<size_t>(
        client->GetConfig().main_config_.find(BODY)->second.size());
  } else if (client->GetStage() == RES_CHUNK &&
             client->GetChunkSize() > client->GetResponse().entity_length_) {
    send_msg_len =
        client->GetChunkSize() % client->GetResponse().entity_length_;
  } else if (client->GetStage() == RES_FIN) {
    send_msg_len = 5;
  } else
    send_msg_len = client->GetMessageLength();
  return (send_msg_len);
}

inline void SendChunkProcess(struct kevent* event, s_client_type* client,
                             const char* send_msg, size_t send_msg_len) {
  if (client->GetSendNum() == DEF) {
    s_stage stage = client->GetStage();
    client->SetSendNum(stage);
  }
  size_t send_len = client->GetSendLength();
  size_t temp_len =
      send(event->ident, send_msg + send_len, send_msg_len - send_len, 0);
  if (temp_len == size_t(-1)) {
    return;
  } else if (temp_len < send_msg_len) {
    client->SetSendLength(temp_len);
    client->SetStage(RES_SEND);
  } else {
    client->SetStage(client->GetSendNum());
    client->SetSendLength(0);
    client->SetSendNum(DEF);
    client->SetBuf(NULL);
  }
}

inline void SendProcess(struct kevent* event, s_client_type* client,
                        const char* send_msg, size_t send_msg_len) {
  if (client->GetStage() == RES_CHUNK || client->GetStage() == RES_FIN ||
      client->GetChunked())
    SendChunkProcess(event, client, send_msg, send_msg_len);
  size_t send_len = client->GetSendLength();
  size_t temp_len =
      send(event->ident, send_msg + send_len, send_msg_len - send_len, 0);
  if (temp_len == size_t(-1)) {
    client->SetStage(RES_SEND);
    return;
  } else if (temp_len < send_msg_len) {
    client->SetSendLength(temp_len);
    client->SetStage(RES_SEND);
  } else {
    client->SetSendLength(0);
    if (client->GetSendNum() != DEF) {
      client->SetStage(client->GetSendNum());
      client->SetSendNum(DEF);

    } else
      client->SetStage(RES_FIN);
    client->SetBuf(NULL);
    return;
  }
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
      if (curr_event->flags & EV_ERROR) {
        PrintError(3, WEBSERV, CRITICAL, "kevent running error");
        ;
      } else {
        s_base_type* ft_filter = static_cast<s_base_type*>(curr_event->udata);
        switch (ft_filter->GetType()) {
          case WORK: {
            s_work_type* work_type = static_cast<s_work_type*>(ft_filter);
            if (work_type->GetWorkType() == file) {
              std::cout << "FILE steps"
                        << " / Task FD : " << ft_filter->GetFD() << std::endl;
              if (work_type->GetClientStage() == GET_START)
                WorkGet(curr_event);
              else if (work_type->GetClientStage() == POST_START)
                WorkFilePost(curr_event);
            } else if (work_type->GetWorkType() == cgi)
              WorkCGIPost(curr_event);
            // std::cout << "cgi steps" << std::endl;

          } break;
          case CLIENT: {
            if (curr_event->filter == EVFILT_READ) {
              {
                std::cout << "READ steps"
                          << " / Task FD : " << ft_filter->GetFD() << std::endl;
                char* client_msg = new char[curr_event->data];
                int ret =
                    recv(curr_event->ident, client_msg, curr_event->data, 0);
                if (ret == -1) {
                  // 임시
                }

                request_handler(curr_event->data, curr_event->udata,
                                client_msg);
                delete[] client_msg;
              }
              switch (static_cast<s_client_type*>(ft_filter)->GetStage()) {
                case GET_READY: {
                  ClientGet(curr_event);
                  break;
                }
                case POST_READY: {
                  if (static_cast<s_work_type*>(ft_filter)->GetWorkType() ==
                      file)
                    ClientFilePost(curr_event);
                  else
                    ClientCGIPost(curr_event);
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
            } else if (curr_event->filter == EVFILT_WRITE) {
              s_client_type* client = static_cast<s_client_type*>(ft_filter);
              std::cout << "WRITE steps"
                        << " / Task FD : " << ft_filter->GetFD() << std::endl;
              char* msg_top;

              if (client->GetStage() == RES_SEND) {
                SendProcess(curr_event, client, client->GetBuf(),
                            client->GetMessageLength());
              } else if (client->GetStage() != RES_FIN) {
                if (client->GetStage() != RES_CHUNK) {
                  client->SetResponse();
                  msg_top = MaketopMessage(client);
                } else
                  msg_top = strdup("");
                client->SetBuf(MakeSendMessage(client, msg_top));
                delete msg_top;
                size_t send_msg_len = ChunkEncoding(client);
                SendProcess(curr_event, client, client->GetBuf(), send_msg_len);
              }
              if (client->GetStage() == RES_SEND)
                ;
              else if (!client->GetChunked() ||
                       client->GetStage() != RES_CHUNK) {
                client->SendLogs();
                if (client->GetStage() == END) {
                  ServerConfig::ChangeEvents(curr_event->ident, EVFILT_WRITE,
                                             EV_DELETE, 0, 0, 0);
                  close(curr_event->ident);
                  ResetConnection(
                      static_cast<s_client_type*>(curr_event->udata));
                } else {
                  ServerConfig::ChangeEvents(curr_event->ident, EVFILT_WRITE,
                                             EV_DISABLE, 0, 0, 0);
                  ServerConfig::ChangeEvents(curr_event->ident, EVFILT_READ,
                                             EV_ENABLE, 0, 0, ft_filter);
                  ResetConnection(
                      static_cast<s_client_type*>(curr_event->udata));
                }
              }
            } else if (curr_event->filter == EVFILT_TIMER ||
                       curr_event->flags & EV_EOF) {
              DeleteUdata(ft_filter);
              ServerConfig::ChangeEvents(curr_event->ident, EVFILT_WRITE,
                                         EV_DELETE, 0, 0, 0);
              ServerConfig::ChangeEvents(curr_event->ident, EVFILT_READ,
                                         EV_DELETE, 0, 0, 0);
              ServerConfig::ChangeEvents(curr_event->ident, EVFILT_TIMER,
                                         EV_DELETE, 0, 0, 0);
              close(curr_event->ident);
            }
          } break;
          case LOGGER: {
            if (curr_event->filter == EVFILT_WRITE) {
              std::cout
                  << "Logger steps"
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
            ServerConfig::ChangeEvents(client_fd, EVFILT_READ, EV_ADD | EV_EOF,
                                       0, 0, client);

            ServerConfig::ChangeEvents(client_fd, EVFILT_WRITE,
                                       EV_ADD | EV_DISABLE, 0, 0, client);
            int timer = atoi(client->GetConfig()
                                 .main_config_.at(TIMEOUT)
                                 .c_str());  // refactoring
            if (timer != 0) {
              ServerConfig::ChangeEvents(client_fd, EVFILT_TIMER, EV_ADD,
                                         NOTE_SECONDS, timer, client);
            }
          } break;
        }
      }
      CheckError(curr_event);
    }
  }
  return;
}
