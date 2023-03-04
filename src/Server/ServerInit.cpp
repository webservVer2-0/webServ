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

bool method_check(const std::string& line) {
  const std::string method[5] = {"GET", "PUT", "POST", "HEAD", "DELETE"};

  for (int i = 0; i < 5; ++i) {
    if (line == method[i]) {
      return (false);
    }
  }
  return (true);
}

bool vector_checker(std::vector<std::string>& lines, std::string to_find) {
  std::vector<std::string>::iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    if (it->find(to_find) != std::string::npos) {
      return (false);
    }
  }
  return (true);
}

bool colon_exist(std::string line) {
  size_t pos = line.find(":");
  if (pos == std::string::npos) {
    return (false);
  }
  return (true);
}

bool colon_checker(std::vector<std::string>& lines) {
  std::vector<std::string>::iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    if (colon_exist(*it)) {
      return (false);
    }
  }
  return (true);
}

t_error valid_check(std::string line, std::vector<std::string> lines,
                    std::vector<std::string> tokens) {
  if (line.empty()) {
    return (BAD_REQ);
  }
  std::string::size_type pos = line.find("\r\n\r\n");
  if (pos == std::string::npos) {
    return (BAD_REQ);
  }
  if (method_check(tokens[0])) {
    return (NOT_IMPLE);
  }
  try {
    if (tokens.at(2) != "HTTP/1.1") {
      return (OLD_HTTP);
    }
  } catch (std::out_of_range) {
    return (BAD_REQ);
  }
  if (vector_checker(lines, "Host:")) {
    return (BAD_REQ);
  }
  if (vector_checker(lines, ":")) {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

std::vector<std::string> msg_tokenizer(char* const client_msg) {
  std::stringstream ss(client_msg);
  std::vector<std::string> tokens;
  std::string line;
  while (ss >> line) {
    tokens.push_back(line);
  }
  return (tokens);
}

std::vector<std::string> msg_liner(char* const client_msg) {
  std::stringstream ss(client_msg);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ss, line)) {
    lines.push_back(line);
  }
  return (lines);
}

t_error fill_init_line(t_html* storage, std::vector<std::string> tokens) {
  try {
    /* method, target */
    // TODO: tokens[1]의 경우, localhost::8080와 같으면 uri로 치환해주기
    storage->init_line_[tokens[0]] = tokens[1];
    if (tokens[2].find("/") != 4 || tokens[2].length() != 8) {
      return (BAD_REQ);
    }
    std::string http = tokens[2].substr(0, 4);
    std::string ver = tokens[2].substr(4, 6);
    if (http != "HTTP" || ver != "1.1") {
      return (BAD_REQ);
    }
    storage->init_line_[http] = ver;
    return (NO_ERROR);
  } catch (std::out_of_range) {
    return (BAD_REQ);
  }
}

t_error fill_header_and_entity(t_html* html, std::string line) {
  std::string::size_type pos = line.find("\r\n\r\n");
  std::string::size_type endpos = line.find("\r\n");
  std::string header_lines = line.substr(endpos + 2, pos - endpos - 2);

  std::string::size_type last_pos = 0;
  std::string::size_type next_pos = 0;
  std::string::size_type colon_pos = 0;
  std::string no_escape_line;
  std::string key;
  std::string value;
  try {
    while (last_pos != header_lines.size()) {
      // 한 줄씩 파싱
      next_pos = header_lines.find("\r\n", last_pos);
      if (next_pos == std::string::npos) {
        next_pos = header_lines.size();
      }

      // \r\n 제거한 라인
      no_escape_line = header_lines.substr(last_pos, next_pos - last_pos);

      // 콜론을 찾아서 key-value로 분리
      colon_pos = no_escape_line.find(":");
      if (colon_pos != std::string::npos) {
        key = line.substr(0, colon_pos);
        value = line.substr(colon_pos + 2);  // 콜론과 스페이스 제외
        html->header_[key] = value;
      } else {
        return (BAD_REQ);
      }
      last_pos = next_pos + 2;
    }
  } catch (std::exception) {
    return (NO_ERROR);
  }
  return (NO_ERROR);
}

t_error request_error(s_client_type* client_type, t_error err_code) {
  client_type->SetErrorCode(err_code);
  client_type->SetStage(REQ_FIN);
  return (err_code);
}

t_error request_msg(void* udata, char* client_msg) {
  /* set REQ_READY */
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);

  /* for parsing, and fill t_html */
  t_html* storage = &client_type->GetRequest();
  t_error error_code = NO_ERROR;
  std::string line(client_msg);
  std::vector<std::string> lines = msg_liner(client_msg);
  std::vector<std::string> tokens = msg_tokenizer(client_msg);

  if ((error_code = valid_check(line, lines, tokens))) {
    return (request_error(client_type, error_code));
  }
  if ((error_code = fill_init_line(storage, tokens))) {
    return (request_error(client_type, error_code));
  }
  if ((error_code = fill_header_and_entity(storage, line))) {
    return (request_error(client_type, error_code));
  }
  client_type->SetStage(REQ_FIN);
  return (error_code);
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
                // TODO : http message valid check
                if (request_msg(curr_event->udata, client_msg)) {
                  // TODO : error();
                } else {
                  ChangeEvents(config.change_list_, curr_event->ident,
                               EVFILT_READ, EV_DELETE, 0, 0, NULL);
                }
                DeleteUdata(static_cast<s_base_type*>(curr_event->udata));
                close(curr_event->ident);
                config.change_list_.clear();
                delete[] client_msg;
                // TODO: 작업 END시 처리해줘야 할 것들은 다음과 같다.
                // TODO: client udata ~ file udata 까지 찾아들어가서 delete 를
                // 진행하면 될듯
              } else if (curr_event->filter == EVFILT_WRITE) {
                std::cout << " client Write step" << std::endl;
              } else if (curr_event->filter == EVFILT_TIMER) {
                // TODO: time out 상태, 적절한 closing 필요
              }
              // eof 로 확인하기는 불확실한 방법으로 판단됨
              //   else if (curr_event->flags == EV_EOF) {
              //     // 임시 삭제
              //     std::cout << "EOF Error, ID 삭제" << std::endl;
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
    }
  }
  return;
}
