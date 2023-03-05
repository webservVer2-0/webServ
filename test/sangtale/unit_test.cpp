#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../../include/webserv.hpp"

void show_vector(std::vector<std::string> v) {
  std::vector<std::string>::iterator it;
  for (it = v.begin(); it != v.end(); ++it) {
    std::cout << *it << "\n";
  }
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
    // show_vector(tokens);
    // std::cout << "1\n";
    storage->init_line_[tokens[0]] = tokens[1];
    if (tokens[2].find("/") != 4 || tokens[2].length() != 8) {
      return (BAD_REQ);
    }
    // std::cout << "2\n";
    std::string http = tokens[2].substr(0, 4);
    std::string ver = tokens[2].substr(5, 6);
    // std::cout << "3\n";
    if (http != "HTTP" || ver != "1.1") {
      return (BAD_REQ);
    }
    // std::cout << "4\n";
    storage->init_line_[http] = ver;
    // std::cout << "5\n";
    return (NO_ERROR);
  }
  /* substr()의 catch */
  catch (std::out_of_range) {
    printf("o o r\n");
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

void fill_entity(t_html* h, const char* msg, std::string line) {
  const char* rq_body = "Request body: ";
  size_t header_end = line.find(rq_body);
  if (header_end == std::string::npos) {
    h->entity = nullptr;
    h->entity_length_ = 0;
    return;
  }
  size_t entity_start = header_end + strlen(rq_body);
  size_t entity_len = strlen(msg) - entity_start;
  char* entity = new char[entity_len + 1];
  memcpy(entity, msg + entity_start, entity_len);
  entity[entity_len] = '\0';

  printf("entity\n");
  printf("%s", entity);
  delete[] entity;
  return;
}

int test_func(t_html* rq_msg) {
  t_error error_code = NO_ERROR;
  char client_msg[] =
      "GET /index.html HTTP/1.1\r\nHost: "
      "www.example.com\r\nConnection: "
      "close\r\n\r\nRequest body: hello world";
  std::string r_line =
      "GET /index.html HTTP/1.1\r\nHost: "
      "www.example.com\r\nConnection: "
      "close\r\n\r\nRequest body: hello world";
  std::string line =
      "GET /index.html HTTP/1.1\r\nHost: "
      "www.example.com\r\nConnection: "
      "close\r\n\r\n";
  std::vector<std::string> lines = msg_liner(client_msg);
  std::vector<std::string> tokens = msg_tokenizer(client_msg);

  if ((error_code = valid_check(client_msg, line, lines, tokens))) {
    return (printf("error1\n"));
  }
  if ((error_code = fill_init_line(rq_msg, tokens))) {
    return (printf("error2\n"));
  }
  if ((error_code = fill_header_and_entity(rq_msg, client_msg, line))) {
    return (printf("error3\n"));
  }
  return (error_code);
}

void show_map(std::map<std::string, std::string> m) {
  std::map<std::string, std::string>::iterator it;
  for (it = m.begin(); it != m.end(); ++it) {
    std::cout << "key: " << it->first << "\nvalue: " << it->second << std::endl;
  }
}

int main(void) {
  t_html h;
  t_html h2;
  const char* R =
      "GET /index.html HTTP/1.1\r\nHost: "
      "www.example.com\r\nConnection: "
      "close\r\n\r\nRequest body: hello world";
  std::string r_line =
      "GET /index.html HTTP/1.1\r\nHost: "
      "www.example.com\r\nConnection: "
      "close\r\n\r\nRequest body: hello world";
  std::string line =
      "GET /index.html HTTP/1.1\r\nHost: "
      "www.example.com\r\nConnection: "
      "close\r\n\r\n";
  test_func(&h);
  show_map(h.init_line_);
  show_map(h.header_);
  // test_func(&h2, R, r_line);
  return 0;
}
