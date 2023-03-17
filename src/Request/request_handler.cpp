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
  bool _exist_content_length;
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
std::string make_uri_path(std::vector<std::string> &vec) {
  std::string ret;

  ret.append(".");
  for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();
       ++it) {
    ret.append("/" + *it);
  }
  return ret;
}

t_error convert_uri(std::string rq_uri,
                    std::map<std::string, t_loc *> location_config,
                    s_client_type &client) {
  std::vector<std::string> rq_path(1);
  size_t pos;
  std::map<std::string, t_loc *>::iterator loc_it;

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
  const std::string requisite[2] = {"GET", "POST"};

  for (int i = 0; i < 2; ++i) {
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
    size_t entity_start = e->_header_crlf + DOUBLE_CRLF_LEN;

    http->entity_ = new char[entity_len + 1];
    memcpy(http->entity_, client_msg + entity_start, entity_len);
    http->entity_[entity_len + 1] = '\0';
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
      if (key == "Content-Length") {
        e->_exist_content_length = true;
      }
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

void set_status(s_client_type* client, t_elem* e) {
  if (e->_method == GET)
    client->SetStage(GET_READY);
  else if (e->_method == POST)
    client->SetStage(POST_READY);
  else if (e->_method == DELETE)
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
  memset(e, 0, sizeof(t_elem));

  if (line.empty()) {
    return (BAD_REQ);
  }
  size_t init_crlf = line.find(CRLF);
  if (init_crlf == std::string::npos) {
    return (BAD_REQ);
  }
  e->_header_crlf = line.find(DOUBLE_CRLF);
  if (e->_header_crlf == std::string::npos) {
    return (BAD_REQ);
  }
  e->_header_line =
      line.substr(init_crlf + CRLF_LEN, e->_header_crlf - init_crlf + CRLF_LEN);
  e->_init_line = line.substr(0, init_crlf);
  return (NO_ERROR);
}

t_error request_handler(void* udata, char* msg) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);

  if (!msg) {
    return (request_error(client_type, BAD_REQ, "message is null"));
  }

  t_http* http = &client_type->GetRequest();
  t_elem e;
  t_error err_code = NO_ERROR;
  std::string line(msg);

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
    set_status(client_type, &e);
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
