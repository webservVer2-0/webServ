#include "../include/webserv.hpp"

#define CRLF "\n"
#define DOUBLE_CRLF "\n\n"

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

char* find_entity(char* client_msg) {
  char* entity = strstr(client_msg, DOUBLE_CRLF);
  entity += strlen(DOUBLE_CRLF);
  return (entity);
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

std::string init_line_substr(char* client_msg) {
  std::string init_line(client_msg);
  return init_line;
}

std::string header_line_substr(char* client_msg) {
  std::string header_line(client_msg);
  return header_line;
}

void request_handler(void* udata, char* client_msg) {
  /*
    사전준비
  */
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);
  t_http* rq_msg = &client_type->GetRequest();
  /*
    필요한 변수들
  */
  t_error error_code = NO_ERROR;
  std::string init_line = init_line_substr(client_msg);
  std::string header_line = header_line_substr(client_msg);

  if ((error_code = _valid_check(client_msg))) {
    request_error(client_type, error_code);
    return;
  }
  client_type->SetStage(REQ_FIN);
  return;
}
