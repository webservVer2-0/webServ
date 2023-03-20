#include "../../include/config.hpp"
#include "../../include/webserv.hpp"

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF_LEN 2
#define DOUBLE_CRLF_LEN 4

typedef enum t_method { GET, POST, DELETE } e_method;

typedef struct s_elem {
  std::string _init_line;
  std::string _header_line;
  size_t _content_length;
  size_t _header_crlf; /* "asd\r\n" */
  e_method _method;
  bool _exist_cookie;
} t_elem;

/**
 * @brief 테스트용 출력함수
 *
 * @param vec
 */
// void print_vector_path(std::vector<std::string> &vec) {
//   std::cout << ".";
//   for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();
//        ++it) {
//     std::cout << "/" + *it;
//   }
//   std::cout << std::endl;
// }

// /**/src/Method
//  * @brief 토큰화된 string을 제대로된 경로로 만들어주는 함수
//  *
//  * @param vec 토큰화된 string 벡터
//  * @return std::string
//  */
std::string make_uri_path(std::vector<std::string>& vec) {
  std::string ret;

  ret.append(".");
  for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();
       ++it) {
    ret.append("/" + *it);
  }
  return ret;
}

t_error convert_uri(std::string rq_uri,
                    std::map<std::string, t_loc*> location_config,
                    s_client_type& client) {
  std::vector<std::string> rq_path(1);
  size_t pos;
  std::map<std::string, t_loc*>::iterator loc_it;

  std::string token = rq_uri;
  while ((pos = rq_uri.find('/')) != std::string::npos) {
    token = rq_uri.substr(0, pos);
    if (token != "" && token != ".") rq_path.push_back(token);
    rq_uri.erase(0, pos + 1);
  }
  if (rq_uri != "" && rq_uri != ".") rq_path.push_back(rq_uri);
  loc_it = location_config.find(("/" + rq_path.front()));
  rq_path.front() = (*loc_it).second->main_config_[ROOT];
  if (rq_path.size() == 1)
    rq_path.push_back((*loc_it).second->main_config_[DEFFILE]);

  if ((*loc_it).second->main_config_[METHOD].find(
          client.GetRequest().init_line_["METHOD"]) == std::string().npos) {
    return FORBID;
  }
  client.SetConfigPtr((*loc_it).second);
  client.GetRequest().init_line_["URI"] = make_uri_path(rq_path);
  // print_vector_path(rq_path);
  return OK;
}

template <typename T>
std::string to_string(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

/**
 * @brief 에러 발생하면 client_type의
 *        t_stage stage_, t_error status_code_ 수정
 *
 * @param client_type udata를 casting한 s_client_type*
 * @param err_code 발생한 에러 코드
 * @return t_error 발생한 에러
 */
static t_error request_error(s_client_type* client_type, t_error err_code,
                             std::string msg) {
  int errno_ = errno;
  client_type->SetError(errno_, msg);
  client_type->SetErrorCode(err_code);
  client_type->SetStage(ERR_READY);
  return (err_code);
}

int method_parser(t_elem* e) {
  const std::string requisite[3] = {"GET", "POST", "DELETE"};

  for (int i = 0; i < 3; ++i) {
    if (e->_init_line.find(requisite[i]) != std::string::npos) {
      e->_method = static_cast<e_method>(i);
      return (0);
    }
  }
  return (1);
}

t_error init_line_parser(t_http* http, t_elem* e) {
  if (method_parser(e)) {
    return (NOT_IMPLE);
  }
  size_t first_space = e->_init_line.find(" ");
  if (first_space == std::string::npos) {
    return (BAD_REQ);
  }
  try {
    http->init_line_["METHOD"] = e->_init_line.substr(0, first_space);
  } catch (std::exception) {
    return (SYS_ERR);
  }

  size_t second_space = e->_init_line.find(" ", first_space + 1);
  if (second_space == std::string::npos) {
    return (BAD_REQ);
  }
  /*
    1. t_http의 init_line_["URI"] 초기화
    2. URI에 /delete?가 있으면, init_line_["METHOD"]와 e->_method를 DELETE로
  */
  try {
    http->init_line_["URI"] =
        e->_init_line.substr(first_space + 1, second_space - first_space - 1);
    if (e->_method == GET &&
        http->init_line_["URI"].find("/delete?") != std::string::npos) {
      http->init_line_["METHOD"] = "DELETE";
      e->_method = DELETE;
    }
  } catch (std::exception) {
    return (SYS_ERR);
  }
  std::string http_ver;
  try {
    http_ver = e->_init_line.substr(second_space + 1,
                                    e->_init_line.size() - second_space + 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  if (http_ver == "HTTP/0.9" || http_ver == "HTTP/1.0") {
    return (OLD_HTTP);
  }
  if (http_ver != "HTTP/1.1") {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

t_error alloc_entity(t_http* http, t_elem* e, char* client_msg) {
  ssize_t entity_len;
  try {
    entity_len = std::stol(http->header_["Content-Length"]);
  } catch (std::invalid_argument const& ex) {
    return (BAD_REQ);
  }
  if (entity_len <= 0) {
    return (BAD_REQ);
  }
  http->entity_length_ = entity_len;
  try {
    const size_t entity_start = e->_header_crlf + DOUBLE_CRLF_LEN;
    http->entity_ = new char[entity_len];
    memcpy(http->entity_, client_msg + entity_start, entity_len);
    return (NO_ERROR);
  } catch (std::bad_alloc& e) {
    return (SYS_ERR);
  }
}

t_error alloc_entity(t_http* http, t_elem* e) {
  ssize_t entity_len;
  try {
    entity_len = std::stol(http->header_["Content-Length"]);
    if (entity_len <= 0) {
      return (BAD_REQ);
    }
  } catch (std::invalid_argument const& ex) {
    return (BAD_REQ);
  }
  if (entity_len <= 0) {
    return (BAD_REQ);
  }
  http->entity_length_ = entity_len;
  try {
    const size_t entity_start = e->_header_crlf + DOUBLE_CRLF_LEN;

    http->entity_ = new char[entity_len];
    memcpy(http->entity_, http->msg.data() + entity_start, entity_len);
    return (NO_ERROR);
  } catch (std::bad_alloc& e) {
    return (SYS_ERR);
  }
}

/**
 * @brief
 *
 * @param http
 * @param e
 * @return t_error
 */
t_error fill_header(t_http* http, t_elem* e) {
  size_t pos = 0;
  size_t end_pos;
  std::string key;
  std::string value;

  while ((end_pos = e->_header_line.find(CRLF, pos)) != std::string::npos) {
    std::string line = e->_header_line.substr(pos, end_pos - pos);
    size_t colon_pos = line.find(": ");

    if (colon_pos != std::string::npos) {
      key = line.substr(0, colon_pos);
      value = line.substr(colon_pos + CRLF_LEN);
      if (key == "Cookie") {
        e->_exist_cookie = true;
      }
      http->header_[key] = value;
    } else if (pos == end_pos) {
      break;
    } else {
      return (BAD_REQ);
    }
    pos = end_pos + CRLF_LEN;
  }
  return (NO_ERROR);
}

void set_status(s_client_type* client, t_http* http) {
  const std::string method = http->init_line_["METHOD"];

  if (method == "GET")
    client->SetStage(GET_READY);
  else if (method == "POST")
    client->SetStage(POST_READY);
  else if (method == "DELETE")
    client->SetStage(DELETE_READY);
  return;
}

/**
 * @brief elem의 멤버 초기화 및 간단한 에러 체크
 *
 * @param e    쓸 정보 담기
 * @param line recv()로 받은 메시지를 string으로 치환
 * @return t_error
 */
t_error elem_initializer(t_elem* e, std::string line) {
  if (line.empty()) {
    return (BAD_REQ);
  }

  const size_t init_crlf = line.find(CRLF);
  if (init_crlf == std::string::npos) {
    return (BAD_REQ);
  }
  memset(e, 0, sizeof(t_elem));
  e->_header_crlf = line.find(DOUBLE_CRLF);
  if (e->_header_crlf == std::string::npos) {
    return (BAD_REQ);
  }
  e->_init_line = line.substr(0, init_crlf);
  e->_header_line =
      line.substr(init_crlf + CRLF_LEN, e->_header_crlf - init_crlf + CRLF_LEN);
  return (NO_ERROR);
}

t_error elem_initializer(t_elem* e, std::vector<char> msg) {
  if (msg.empty()) {
    return (BAD_REQ);
  }

  std::vector<char>::iterator double_crlf_pos =
      finderToVector(msg, DOUBLE_CRLF);
  // double_crlf가 없으면
  if (double_crlf_pos == msg.end()) {
    return (BAD_REQ);
  }
  memset(e, 0, sizeof(t_elem));
  std::string top(msg.begin(), double_crlf_pos + DOUBLE_CRLF_LEN);

  const size_t init_crlf = top.find("\r\n");
  e->_init_line = top.substr(0, init_crlf);
  e->_header_crlf = std::distance(msg.begin(), double_crlf_pos);
  e->_header_line =
      top.substr(init_crlf + CRLF_LEN, top.size() - (init_crlf + CRLF_LEN));
  return (NO_ERROR);
}

t_error request_handler(size_t msg_len, void* udata, char* msg) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);

  if (!msg) {
    return (request_error(client_type, BAD_REQ, "message is null"));
  }

  t_http* http = &client_type->GetRequest();
  t_elem e;
  t_error err_code = NO_ERROR;
  std::string line;

  for (size_t i = 0; i < msg_len; i += 2) {
    line.push_back(msg[i]);
    if (i + 1 < msg_len) {
      line.push_back(msg[i + 1]);
    }
  }

  // TODO: msg 길이에 널이 안들어감!!!!

  try {
    err_code = elem_initializer(&e, line);
    if (err_code) {
      return (request_error(client_type, err_code, "elem_initializer()"));
    }
    err_code = init_line_parser(http, &e);
    if (err_code) {
      return (request_error(client_type, err_code, "init_line_parser()"));
    }
    if (fill_header(http, &e)) {
      return (request_error(client_type, BAD_REQ, "fill_header()"));
    }
    if (e._exist_cookie) {
      client_type->SetCookieId(http->header_["COOKIE"]);
    }
    if (e._method == POST) {
      if ((err_code = alloc_entity(http, &e, msg))) {
        return (request_error(client_type, err_code, "alloc()"));
      }
    }
    set_status(client_type, http);
    err_code = convert_uri(
        http->init_line_["URI"],
        client_type->GetParentServer().GetServerConfig().location_configs_,
        *client_type);
  } catch (std::exception& e) {
    return (request_error(client_type, BAD_REQ,
                          "request_handler() exception occur"));
  }
  return (err_code);
}

/*
***
    이후 구현한 파트
***
*/

t_error baking_cookie(s_client_type* client_type, t_http* http) {
  std::string old_cookie = client_type->GetCookieId();
  std::string new_cookie = http->header_["Cookie"];
  size_t equal_pos = new_cookie.find("=");
  if (equal_pos == std::string::npos) {
    return (BAD_REQ);
  }
  // "id=" 이후로 지우고
  new_cookie.erase(equal_pos + 1, new_cookie.size() - (equal_pos + 1));
  // 이전 cookie 삽입
  new_cookie += old_cookie;
  http->header_["Cookie"] = new_cookie;
  client_type->SetCookieId(old_cookie);
  return (NO_ERROR);
}

inline std::vector<char> stringToCharVector(std::string line) {
  std::vector<char> ret;
  for (int i = 0; i < line.size(); ++i) {
    ret.push_back(line[i]);
  }
  return (ret);
}

inline std::vector<char>::iterator finderToVector(std::vector<char> msg,
                                                  std::string line) {
  std::vector<char> double_crlf = stringToCharVector(line);
  std::vector<char>::iterator result = std::search(
      msg.begin(), msg.end(), double_crlf.begin(), double_crlf.end());
  return (result);
}

t_error post_handler(s_client_type* client_type, t_http* http) {
  t_elem e;
  t_error err_code = NO_ERROR;
  try {
    err_code = elem_initializer(&e, http->msg);
    if (err_code) {
      return (request_error(client_type, err_code, "elem_initializer()"));
    }
    err_code = init_line_parser(http, &e);
    if (err_code) {
      return (request_error(client_type, err_code, "elem_initializer()"));
    }
    if (fill_header(http, &e)) {
      return (request_error(client_type, BAD_REQ, "fill_header()"));
    }
    if (e._exist_cookie) {
      err_code = baking_cookie(client_type, http);
      if (err_code) {
        return (request_error(client_type, err_code, "baking cookie()"));
      }
    }
    err_code = alloc_entity(http, &e);
    if (err_code) {
      return (request_error(client_type, err_code, "alloc_entity()"));
    }
  } catch (const std::exception& e) {
    return (BAD_REQ);
  }
  return (err_code);
}

t_error get_handler(s_client_type* client_type, t_http* http) {
  std::string line(http->msg.begin(), http->msg.end());
  t_elem e;
  t_error err_code = NO_ERROR;

  try {
    err_code = elem_initializer(&e, line);
    if (err_code) {
      return (request_error(client_type, err_code, "elem_initializer()"));
    }
    err_code = init_line_parser(http, &e);
    if (err_code) {
      return (request_error(client_type, err_code, "elem_initializer()"));
    }
    if (fill_header(http, &e)) {
      return (request_error(client_type, BAD_REQ, "fill_header()"));
    }
    if (e._exist_cookie) {
      err_code = baking_cookie(client_type, http);
      if (err_code) {
        return (request_error(client_type, err_code, "baking cookie()"));
      }
    }

  } catch (const std::exception& e) {
    return (BAD_REQ);
  }
  return (err_code);
}

t_error request_handler(void* udata) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);

  t_http* http = &client_type->GetRequest();
  t_error err_code = NO_ERROR;

  std::string method = http->init_line_["METHOD"];
  try {
    if (method == "POST") {
      err_code = post_handler(client_type, http);
    } else if (method == "GET" || method == "DELETE") {
      err_code = get_handler(client_type, http);
    } else {
      return (request_error(client_type, NOT_IMPLE, "구현하지 않은 메소드"));
    }
    if (err_code) {
      return (request_error(client_type, err_code, "request_handler()"));
    }
    err_code = convert_uri(
        http->init_line_["URI"],
        client_type->GetParentServer().GetServerConfig().location_configs_,
        *client_type);
    if (err_code) {
      return (request_error(client_type, err_code, "request_handler()"));
    }
    set_status(client_type, http);
  } catch (const std::exception& e) {
    return (request_error(client_type, SYS_ERR, "rq_hdlr() 예외 발생"));
  }
  return (err_code);
}
