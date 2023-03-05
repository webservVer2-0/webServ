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

/*************************************************************************
start
**************************************************************************/

/**
 * @brief 요청을 공백과 이스케이프를 구분자로 토큰화
 *
 * @param client_msg 요청 메시지
 * @return std::vector<std::string> 토큰
 */
std::vector<std::string> msg_tokenizer(const char* client_msg) {
  std::stringstream ss(client_msg);
  std::vector<std::string> tokens;
  std::string line;
  while (ss >> line) {
    tokens.push_back(line);
  }
  return (tokens);
}

/**
 * @brief 요청을 이스케이프 문자를 구분자로 토큰화
 *
 * @param client_msg 요청 메시지
 * @return std::vector<std::string> 라인들
 */
std::vector<std::string> msg_liner(const char* client_msg) {
  std::stringstream ss(client_msg);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ss, line)) {
    lines.push_back(line);
  }
  return (lines);
}

/**
 * @brief  string에서 지원하는 메소드를 찾으면 false
 *
 * @param line    타겟
 * @return true   지원하는 메소드가 없으면
 * @return false  지원하는 메소드가 있으면
 */
bool method_check(const std::string& line) {
  const std::string method[5] = {"GET", "PUT", "POST", "HEAD", "DELETE"};

  for (int i = 0; i < 5; ++i) {
    if (line == method[i]) {
      return (false);
    }
  }
  return (true);
}

/**
 * @brief 벡터에 to_find가 있으면 false 반환
 *
 * @param lines     : 타겟 벡터
 * @param to_find   : 찾고 싶은 string
 * @return true     : to_find가 없으면
 * @return false    : to_find가 있으면
 */
bool host_check(std::vector<std::string>& lines) {
  std::vector<std::string>::iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    if (it->find("Host: ") != std::string::npos) {
      return (false);
    }
  }
  return (true);
}

/**
 * @brief line에서 header 라인 부분만 반환
 *
 * @param line : 다듬을 라인
 * @return header_line
 */
std::string refine_header_line(std::string line) {
  size_t header_start = line.find("\r\n") + 2;
  size_t header_end = line.find("\r\n\r\n");
  std::string header_line =
      line.substr(header_start, header_end - header_start);
  return (header_line);
}

/**
 * @brief 헤더 라인에 콜론 체크하고, 콜론이 존재하지 않으면 true 반환
 *
 * @param lines client_msg
 * @return true 콜론이 존재하지 않으면
 * @return false 콜론이 존재하면
 */
bool colon_check(std::string line) {
  std::string header_line = refine_header_line(line);
  const char* hd_line = header_line.c_str();
  std::vector<std::string> lines = msg_liner(hd_line);
  std::vector<std::string>::iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    if (it->find(": ") == std::string::npos) {
      return (true);
    }
  }
  return (false);
}

/**
 * @brief 유효성 검사
 *        1. 빈 요청 : BAD_REQ
 *        2. 유효한 양식(헤더와 바디 사이의 개행 2번) : BAD_REQ
 *        3. 메소드 검사 : NOT_IMPLE
 *        4. HTTP 여부/버전 검사 : OLD_HTTP
 *        5. HTTP
 *
 * @param line
 * @param lines
 * @param tokens
 * @return t_error
 */
t_error valid_check(char* client_msg, std::string& line,
                    std::vector<std::string>& lines,
                    std::vector<std::string>& tokens) {
  if (line.empty()) {
    return (BAD_REQ);
  }
  size_t pos = line.find("\r\n\r\n");
  if (pos == std::string::npos) {
    return (BAD_REQ);
  }
  if (method_check(tokens[0])) {
    return (NOT_IMPLE);
  }
  /* at(), substr()의 예외 throw */
  try {
    if (tokens.at(2).substr(0, 4) != "HTTP") {
      return (BAD_REQ);
    }
    if (tokens.at(2) != "HTTP/1.1") {
      return (OLD_HTTP);
    }
  }
  /* at(), substr() -> catch */
  catch (std::out_of_range) {
    return (BAD_REQ);
  }
  /* Host check */
  if (host_check(lines)) {
    return (BAD_REQ);
  }
  if (colon_check(line)) {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

/**
 * @brief
 *
 * @param storage
 * @param tokens
 * @return t_error
 */
t_error fill_init_line(t_html* storage, std::vector<std::string> tokens) {
  /* substr()의 throw */
  try {
    /* method, target */
    // TODO: tokens[1]의 경우, localhost::8080와 같으면 uri로 치환해주기
    storage->init_line_[tokens[0]] = tokens[1];
    if (tokens[2].find("/") != 4 || tokens[2].length() != 8) {
      return (BAD_REQ);
    }
    std::string http = tokens[2].substr(0, 4);
    std::string ver = tokens[2].substr(5, 6);
    if (http != "HTTP" || ver != "1.1") {
      return (BAD_REQ);
    }
    storage->init_line_[http] = ver;
    return (NO_ERROR);
  }
  /* substr()의 catch */
  catch (std::out_of_range) {
    return (BAD_REQ);
  }
}

/**
 * @brief       header_line을 순회하며 html의 header_를 초기화하는 재귀함수
 *
 * @param html : html의 header_를 초기화
 * @param line : "\r\n"이 포함되거나 포함되지 않은 라인
 *               "\r\n"이 포함되지 않으면 map에 넣고 재귀 탈출
 * @param s : start
 * @param e : end
 */
void recursive_fill_header(t_html* html, std::string line, size_t s, size_t e) {
  bool last_line = false;
  std::string no_escape_line;
  size_t colon_pos;

  e = line.find("\r\n", s);
  if (e == std::string::npos) {
    e = line.size();
    last_line = true;
  }
  no_escape_line = line.substr(s, e - s);
  colon_pos = no_escape_line.find(":");
  if (colon_pos != std::string::npos) {
    std::string key = no_escape_line.substr(0, colon_pos);
    std::string value = no_escape_line.substr(colon_pos + strlen(": "));
    html->header_[key] = value;
  }
  if (last_line) {
    return;
  }
  recursive_fill_header(html, line, s + e + strlen("\r\n"), e);
}

/**
 * @brief
 *
 * @param rq_msg      entity를 초기화할 s_client_type의 멤버 request_msg_
 * @param client_msg  recv한 메시지
 * @param line        string으로 형변환한 client_msg
 * @return t_error    동적 할당에 실패하면 SYS_ERR 반환
 */
t_error malloc_n_copy_entity(t_html* rq_msg, char* client_msg,
                             std::string line) {
  const char* rq_body = "Request body: ";
  size_t header_end = line.find(rq_body);
  if (header_end == std::string::npos) {
    rq_msg->entity = nullptr;
    rq_msg->entity_length_ = 0;
    return (NO_ERROR);
  }
  size_t entity_start = header_end + strlen(rq_body);
  rq_msg->entity_length_ = strlen(client_msg) - entity_start;
  rq_msg->entity = new char[rq_msg->entity_length_ + 1];
  if (rq_msg->entity == nullptr) {
    return (SYS_ERR);
  }
  memcpy(rq_msg->entity, client_msg + entity_start, rq_msg->entity_length_);
  rq_msg->entity[rq_msg->entity_length_] = '\0';
  return (NO_ERROR);
}

t_error fill_header_and_entity(t_html* html, char* client_msg,
                               std::string line) {
  std::string header_line = refine_header_line(line);

  /* entity 체크 */
  if (line.find("\r\n\r\n") + strlen("\r\n\r\n") < line.size()) {
    /* char* entity_ 동적 할당 + memcpy로 복제, entity_length 초기화 */
    if (malloc_n_copy_entity(html, client_msg, line)) {
      return (SYS_ERR);
    }
  }
  /* 재귀 돌면서 header line을 map에 담음 */
  recursive_fill_header(html, header_line, 0, 0);
  return (NO_ERROR);
}

/**
 * @brief 에러 발생하면 client_type의
 *        t_stage stage_, t_error status_code_ 수정
 *
 * @param client_type udata를 casting한 s_client_type*
 * @param err_code 발생한 에러 코드
 * @return t_error 발생한 에러
 */
t_error request_error(s_client_type* client_type, t_error err_code) {
  client_type->SetErrorCode(err_code);
  client_type->SetStage(REQ_FIN);
  return (err_code);
}

/**
 * @brief 1. REQ_READY
 *        2. 유효성 검사
 *        3. init_line 채우기
 *        4. header 채우기
 *        5. entity 있으면 entity 채우기
 *        6. REQ_FIN, 에러 발생하면 에러코드, 아니면 NO_ERROR 반환
 *
 * @param udata       kevent()과 관리하는 udata, client_type으로 캐스팅함
 * @param client_msg  recv()로 받은 client_mas
 * @return t_error 에러 코드
 */
t_error request_msg(void* udata, char* client_msg) {
  /* set REQ_READY */
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);

  /* for parsing, and fill request_msg */
  t_html* rq_msg = &client_type->GetRequest();
  t_error error_code = NO_ERROR;
  std::string line(client_msg);
  std::vector<std::string> lines = msg_liner(client_msg);
  std::vector<std::string> tokens = msg_tokenizer(client_msg);
  if ((error_code = valid_check(client_msg, line, lines, tokens))) {
    return (request_error(client_type, error_code));
  }
  if ((error_code = fill_init_line(rq_msg, tokens))) {
    return (request_error(client_type, error_code));
  }
  if ((error_code = fill_header_and_entity(rq_msg, client_msg, line))) {
    return (request_error(client_type, error_code));
  }
  client_type->SetStage(REQ_FIN);
  return (error_code);
}

/*************************************************************************
end
**************************************************************************/

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
