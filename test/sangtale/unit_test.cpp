#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../../include/webserv.hpp"

void vector_print(std::vector<std::string>& vec, std::string vector_name) {
  std::vector<std::string>::iterator it = vec.begin();
  std::cout << "<< " << vector_name << " >>\n";
  int i = 0;
  for (; it != vec.end(); ++it) {
    std::cout << "index[" << i << "] : ";
    std::cout << *it << "\n";
    ++i;
  }
}

std::vector<std::string>::iterator vector_checker(
    std::vector<std::string>& lines, std::string to_find) {
  std::vector<std::string>::iterator it = lines.begin();
  for (; it != lines.end(); ++it) {
    if (*it == to_find) {
      return (it);
    }
  }
  return (it);
}

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

void debug_print(std::string f, std::string s) {
  std::cout << f << " : " << s << "\n";
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
  std::string::size_type header_end = line.find("\r\n\r\n");
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
    rq_msg->entity = nullptr;
    rq_msg->entity_length_ = 0;
    std::cerr << "entity_ memory allocation failed: " << e.what() << SEND;
    return (SYS_ERR);
  }
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

static t_error fill_entity(t_html* rq_msg, std::string line, char* client_msg) {
  if (malloc_n_copy_entity(rq_msg, client_msg, line)) {
    return (SYS_ERR);
  }
  return (NO_ERROR);
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
  int end_flag = 0;
  std::string no_escape_line;
  size_t colon_pos;

  e = line.find("\r\n", s);
  if (e == std::string::npos) {
    e = line.size();
    end_flag = 1;
  }
  no_escape_line = line.substr(s, e - s);
  colon_pos = no_escape_line.find(":");
  if (colon_pos != std::string::npos) {
    std::string key = no_escape_line.substr(0, colon_pos);
    std::string value = no_escape_line.substr(colon_pos + strlen(": "));
    html->header_[key] = value;
  }
  if (end_flag) return;
  recursive_fill_header(html, line, s + e + strlen("\r\n"), e);
}

void _fill_entity(t_html* h, const char* msg, std::string line) {
  size_t header_end = line.find("Request body: ");
  std::cout << "line :" << line << "\n";
  std::cout << "header_end " << header_end << "\n";
  if (header_end == std::string::npos) {
    h->entity = nullptr;
    h->entity_length_ = 0;
    return;
  }
  size_t entity_start = header_end + 15;
  size_t entity_len = strlen(msg) - entity_start;
  char* entity = new char[entity_len + 1];
  memcpy(entity, msg + entity_start, entity_len);
  entity[entity_len] = '\0';

  printf("entity\n");
  printf("%s", entity);
  delete[] entity;
  return;
}

t_error test_func(t_html* h, const char* msg, std::string line) {
  std::string::size_type header_start = line.find("\r\n") + 2;
  std::string::size_type header_end = line.find("\r\n\r\n");
  std::string header_line =
      line.substr(header_start, header_end - header_start);
  bool entity_exist = false;

  if (header_end + strlen("\r\n\r\n") < line.size()) {
    entity_exist = true;
  }
  recursive_fill_header(h, header_line, 0, 0);
  if (entity_exist) {
    _fill_entity(h, msg, line);
  }
  return (NO_ERROR);
}

void show_map(std::map<std::string, std::string> m) {
  std::map<std::string, std::string>::iterator it;
  for (it = m.begin(); it != m.end(); ++it) {
    std::cout << "key: " << it->first << "\nvalue: " << it->second << std::endl;
  }
}

static bool exist_post_and_entity(std::string line,
                                  std::vector<std::string> tokens) {
  if (tokens[0] == "POST") {
    if (line.find("\r\n\r\n") + strlen("\r\n\r\n") < line.size()) {
      return (true);
    }
  }
  return (false);
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

void test_func2(t_html* h2, char* client_msg, std::string rq_line) {
  /* for parsing, and fill request_msg */
  t_error error_code = NO_ERROR;
  std::string line(client_msg);
  std::vector<std::string> lines = msg_liner(client_msg);
  std::vector<std::string> tokens = msg_tokenizer(client_msg);
  if (exist_post_and_entity(line, tokens)) {
    /* entity_ 동적 할당 */
    if ((error_code = fill_entity(h2, line, client_msg))) {
      std::cout << "error fill_entity()\n";
    }
    /* entity_의 길이와 헤더의 Content-Length 대조 */
    if ((error_code = entity_length_check(h2, lines))) {
      std::cout << "error entity_length_check()\n";
    }
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
  "POST /submit-form HTTP/1.1\r\n";
  char* RQ =
      "POST /index.html HTTP/1.1\r\n"
      "Host : www.example.com\r\n Content-Type: "
      "application/x-www-form-urlencoded\r\nContent-Length: "
      "26\r\n\r\nname=John+Doe&age=25&sex=M";
  std::string rq_line =
      "POST /index.html HTTP/1.1\r\n"
      "Host : www.example.com\r\n Content-Type: "
      "application/x-www-form-urlencoded\r\nContent-Length: "
      "26\r\n\r\nname=John+Doe&age=25&sex=M";
  char d[] = "name=John+Doe&age=25&sex=M";
  test_func2(&h2, RQ, rq_line);
  std::cout << "entity_ : " << h2.entity << "\n";
  std::cout << "entity_length : " << h2.entity_length_ << "\n";
  std::cout << "strlen() : " << strlen(h2.entity) << "\n";
  return 0;
}
