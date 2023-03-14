#include "../include/webserv.hpp"

#define CRLF "\n"
#define DOUBLE_CRLF "\n\n"

/**
 * @brief 요청을 공백과 이스케이프를 구분자로 토큰화
 *
 * @param client_msg 요청 메시지
 * @return std::vector<std::string> 토큰
 */
static std::vector<std::string> msg_tokenizer(const char* client_msg) {
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
static std::vector<std::string> msg_liner(const char* client_msg) {
  std::stringstream ss(client_msg);
  std::vector<std::string> lines;
  std::string line;
  while (std::getline(ss, line)) {
    lines.push_back(line);
  }
  return (lines);
}

/**
 * @brief
 *
 * @param line      client_msg
 * @param tokens    method 체크 하기 위한 tokens
 * @return true
 * @return false
 */

static bool exist_post_and_entity(std::string line,
                                  std::vector<std::string> tokens) {
  if (tokens[0] == "POST") {
    if (line.find(DOUBLE_CRLF) + strlen(DOUBLE_CRLF) < line.size()) {
      return (true);
    }
  }
  return (false);
}

/**
 * @brief  string에서 지원하는 메소드를 찾으면 false
 *
 * @param line    타겟
 * @return true   지원하는 메소드가 없으면
 * @return false  지원하는 메소드가 있으면
 */
static bool method_check(const std::string& line) {
  const std::string method[3] = {"GET", "DELETE", "POST"};

  for (int i = 0; i < 3; ++i) {
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
static bool host_check(std::vector<std::string>& lines) {
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
static std::string refine_header_line(std::string line) {
  // 첫 CRLF 찾은 지점에서 + 2(... HTTP/1.1\n\n<here>)
  const size_t header_start = line.find(CRLF) + 2;
  // 헤더와 entity 라인 사이의 "\n\n" 지점
  const size_t header_end = line.find(DOUBLE_CRLF);
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
static bool colon_check(std::string line) {
  // 헤더 라인만 체크
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
static t_error valid_check(char* client_msg, std::string& line,
                           std::vector<std::string>& lines,
                           std::vector<std::string>& tokens) {
  if (line.empty()) {
    return (BAD_REQ);
  }
  size_t pos = line.find(DOUBLE_CRLF); /* TODO : char* 형태로 고쳐야 함 */
  if (pos == std::string::npos) {
    return (BAD_REQ);
  }
  /* TODO : URI 기반 체크 */
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
  /* 헤더 라인에 colon이 있는지 */
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
static t_error fill_init_line(t_html* storage,
                              std::vector<std::string> tokens) {
  /* substr()의 throw */
  try {
    /* method, target */
    // TODO: tokens[1]의 경우, localhost::8080와 같으면 uri로 치환해주기
    storage->init_line_[tokens[0]] = tokens[1];
    if (tokens[2].find("/") != 4 || tokens[2].length() != 8) {
      return (BAD_REQ);
    }
    /* HTTP, 1.1*/
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
static void recursive_fill_header(t_html* html, std::string line, size_t s,
                                  size_t e) {
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
  try {
    if (colon_pos != std::string::npos) {
      std::string key = no_escape_line.substr(0, colon_pos);
      std::string value = no_escape_line.substr(colon_pos + strlen(": "));
      html->header_[key] = value;
    }
  } catch (std::out_of_range) {
    std::cerr << "recursive_fill_header() out_of_range occur\n";
    return;
  }
  if (last_line) {
    return;
  }
  recursive_fill_header(html, line, s + e + strlen(CRLF), e);
}

/**
 * @brief
 *
 * @param rq_msg      entity를 초기화할 s_client_type의 멤버 request_msg_
 * @param client_msg  recv한 메시지
 * @param line        string으로 형변환한 client_msg
 * @return t_error    동적 할당에 실패하면 SYS_ERR 반환
 */

static t_error malloc_n_copy_entity(t_html* rq_msg, char* client_msg,
                                    std::string line) {
  std::string::size_type header_end = line.find(DOUBLE_CRLF);
  /* bad alloc throw */
  try {
    size_t entity_start = header_end + 4;

    rq_msg->entity_length_ = strlen(client_msg) - entity_start;
    /* 동적 할당 실패시 예외 throw */
    rq_msg->entity = new char[rq_msg->entity_length_ + 1];
    memcpy(rq_msg->entity, client_msg + entity_start, rq_msg->entity_length_);
    /* Null 종료 */
    rq_msg->entity[rq_msg->entity_length_ + 1] = '\0';
    return (NO_ERROR);
  }
  /* bad alloc catch */
  catch (std::bad_alloc& e) {
    rq_msg->entity = NULL;
    rq_msg->entity_length_ = 0;
    std::cerr << "entity_ memory allocation failed: " << e.what() << SEND;
    return (SYS_ERR);
  }
}

/**
 * @brief 재귀돌면서 header line을 request_msg_의 header_line_에 매핑
 *
 * @param rq_msg
 * @param line
 * @return t_error
 */

static t_error fill_header(t_html* rq_msg, std::string line) {
  std::string header_line = refine_header_line(line);
  /* 재귀 돌면서 header line을 map에 담음 */
  recursive_fill_header(rq_msg, header_line, 0, 0);
  return (NO_ERROR);
}

static size_t find_content_length(std::vector<std::string> vec) {
  std::vector<std::string>::iterator it = vec.begin();
  std::string content_length;

  for (; it != vec.end(); ++it) {
    if (it->find("Content-Length: ") != std::string::npos) {
      content_length = *it;
      content_length.erase(0, strlen("Content-Length: "));
      return (std::atoi(content_length.c_str()));
    }
  }
  return (std::string::npos);
}

static t_error entity_length_check(t_html* rq_msg,
                                   std::vector<std::string> lines) {
  const size_t content_len = find_content_length(lines);

  if (content_len == std::string::npos) {
    return (BAD_REQ);
  }
  if (content_len != strlen(rq_msg->entity)) {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

/**
 * @brief entity char* 동적 할당, 할당 실패하면 SYS_ERR return
 *
 * @param rq_msg
 * @param line
 * @param client_msg
 * @return t_error
 */
static t_error fill_entity(t_html* rq_msg, std::string line, char* client_msg) {
  if (malloc_n_copy_entity(rq_msg, client_msg, line)) {
    return (SYS_ERR);
  }
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
static t_error request_error(s_client_type* client_type, t_error err_code) {
  client_type->SetErrorCode(err_code);
  client_type->SetStage(REQ_FIN);
  return (err_code);
}

/*
  new
*/

int method_check(char* msg) {
  if ((memcmp(msg, "GET", 3) == 0)) {
    return (0);
  }
  if ((memcmp(msg, "DELETE", 6) == 0)) {
    return (0);
  }
  if ((memcmp(msg, "POST", 4) == 0)) {
    return (0);
  }
  return (1);
}

int http_check(char* client_msg) {
  const char* offset = strstr(client_msg, "HTTP/1.1");
  if (offset == NULL) {
    return (1);
  }
  return (0);
}

/**
 * @brief ""
 *
 * @param client_msg
 * @return t_error
 */
t_error _valid_check(char* client_msg) {
  if (client_msg == NULL) {
    return (BAD_REQ);
  }
  if (memcmp(client_msg, "", strlen(client_msg)) == 0) {
    return (BAD_REQ);
  }
  if (strstr(client_msg, DOUBLE_CRLF) == NULL) {
    return (BAD_REQ);
  }
  if (method_check(client_msg)) {
    return (NOT_IMPLE);
  }
  if (http_check(client_msg)) {
    return (OLD_HTTP);
  }
  return (NO_ERROR);
}

/**
 * @brief 1. REQ_READY
 *        2. 유효성 검사
 *        3. init_line 채우기
 *        4. header 채우기
 *        5. entity 있으면 entity 채우고, 헤더의 Content-Length와 대조
 *        6. REQ_FIN, 에러 발생하면 에러코드, 아니면 NO_ERROR 반환
 *
 * @param udata       kevent()과 관리하는 udata, client_type으로 캐스팅함
 * @param client_msg  recv()로 받은 client_mas
 * @return t_error 에러 코드
 */
t_error request_msg(void* udata, char* client_msg) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);
  t_error error_code = NO_ERROR;
  t_html* rq_msg = &client_type->GetRequest();

  /* set REQ_READY */

  /* for parsing, and fill request_msg */

  std::string line(client_msg);
  std::vector<std::string> lines = msg_liner(client_msg);
  std::vector<std::string> tokens = msg_tokenizer(client_msg);

  /* TODO : char*로 parsing 해야 함
    size_t header_end = find_header_end();
  */

  if ((error_code = valid_check(client_msg, line, lines, tokens))) {
    return (request_error(client_type, error_code));
  }
  if ((error_code = fill_init_line(rq_msg, tokens))) {
    return (request_error(client_type, error_code));
  }
  if ((error_code = fill_header(rq_msg, line))) {
    return (request_error(client_type, error_code));
  }
  /* method가 post면서 entity가 있으면 */
  if (exist_post_and_entity(line, tokens)) {
    /* t_html* rq_msg->entity_ 동적 할당 */
    if (error_code = fill_entity(rq_msg, line, client_msg)) {
      return (request_error(client_type, error_code));
    }
    /* entity_의 길이와 헤더의 Content-Length 대조 */
    if (error_code = entity_length_check(rq_msg, lines)) {
      return (request_error(client_type, error_code));
    }
  } else {
    rq_msg->entity = NULL;
    rq_msg->entity_length_ = 0;
  }
  client_type->SetStage(REQ_FIN);
  return (error_code);
}

t_error request_message(void* udata, char* client_msg) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);
  t_error error_code = NO_ERROR;
  t_html* rq_msg = &client_type->GetRequest();

  if ((error_code = _valid_check(client_msg))) {
    return (request_error(client_type, error_code));
  }
  if ((error_code = _fill_init_line(client_msg))) {
    return (request_error(client_type, error_code));
  }
  client_type->SetStage(REQ_FIN);
  return (error_code);
}
