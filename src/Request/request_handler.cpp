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
  size_t _header_crlf;
  e_method _method;
} t_elem;

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
  std::vector<std::string> rq_path;
  size_t pos;
  std::map<std::string, t_loc*>::iterator loc_it;

  std::string token = rq_uri;

  std::cout << "origin uri is: " << token << std::endl;

  while ((pos = rq_uri.find('/')) != std::string::npos) {
    token = rq_uri.substr(0, pos);
    if (token != "" && token != ".") rq_path.push_back(token);
    rq_uri.erase(0, pos + 1);
  }
  std::cout << "remain uri is " << rq_uri << std::endl;
  if (rq_uri != ".") rq_path.push_back(rq_uri);
  std::cout << "front is " << rq_path.front() << std::endl;
  loc_it = location_config.find(("/" + rq_path.front()));
  if (loc_it == location_config.end()) {
    loc_it = location_config.find("/");
    rq_path.insert(rq_path.begin(), (*loc_it).second->main_config_[ROOT]);
  } else {
    rq_path.front() = (*loc_it).second->main_config_[ROOT];
  }
  std::cout << "size: " << rq_path.size() << std::endl;
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

/**
 * @brief string을 인자로 받아 vector<char>로 전환
 *
 * @param line 벡터로 만들 string
 * @return string에서 전환된 vector<char>
 */
inline std::vector<char> StringToCharVector(std::string line) {
  std::vector<char> ret;
  for (size_t i = 0; i < line.size(); ++i) {
    ret.push_back(line[i]);
  }
  return (ret);
}

/**
 * @brief vector<char>에서 string이 존재하는 곳의 반복자 반환, 없으면 vector의
 * end()
 *
 * @param vec 대상 벡터
 * @param line 찾고 싶은 string
 * @return std::vector<char>::iterator
 */
inline std::vector<char>::iterator FinderToVector(std::vector<char> vec,
                                                  std::string line) {
  std::vector<char> double_crlf = StringToCharVector(line);
  std::vector<char>::iterator result = std::search(
      vec.begin(), vec.end(), double_crlf.begin(), double_crlf.end());
  return (result);
}

inline void SetPrevCookie(s_client_type* client_type, t_http* http) {
  std::string cookie_line = http->header_["Cookie"];
  size_t equal_pos = cookie_line.find("=");
  if (equal_pos == std::string::npos) return;
  /* 발급 받은 Cookie가 없을 경우. (예: Cookie: id=)*/
  if (equal_pos + 1 == cookie_line.size()) return;
  std::string prev_id =
      cookie_line.substr(equal_pos + 1, cookie_line.size() - equal_pos + 1);
  client_type->SetCookieId(prev_id);
  return;
}

/**
 * @brief 에러 발생하면 client_type의
 *        t_stage stage_, t_error status_code_ 수정
 *
 * @param client_type udata를 casting한 s_client_type*
 * @param err_code 발생한 에러 코드
 * @return t_error 발생한 에러
 */
static t_error RequestError(s_client_type* client_type, t_error err_code,
                            std::string msg) {
  int errno_ = errno;
  client_type->SetErrorString(errno_, msg);
  client_type->SetErrorCode(err_code);
  client_type->SetStage(ERR_READY);
  return (err_code);
}

/**
 * @brief
 *
 * @param e e->_method에 해당하는 method 초기화
 * @return int 메소드가 없으면 error 반환
 */
static int MethodParser(t_elem* e) {
  const std::string requisite[3] = {"GET", "POST", "DELETE"};

  for (int i = 0; i < 3; ++i) {
    if (e->_init_line.find(requisite[i]) != std::string::npos) {
      e->_method = static_cast<e_method>(i);
      return (0);
    }
  }
  return (1);
}

/**
 * @brief 리퀘스트 첫 라인 파싱하고 매핑
 *
 * @param http 초기화할 대상
 * @param e 참조할 속성
 * @return t_error 에러 종류
 */
static t_error InitLineParser(t_http* http, t_elem* e) {
  if (MethodParser(e)) return (NOT_IMPLE);

  size_t first_space = e->_init_line.find(" ");
  if (first_space == std::string::npos) return (BAD_REQ);

  try {
    http->init_line_["METHOD"] = e->_init_line.substr(0, first_space);
  } catch (std::exception) {
    return (SYS_ERR);
  }

  size_t second_space = e->_init_line.find(" ", first_space + 1);
  if (second_space == std::string::npos) return (BAD_REQ);
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
  if (http_ver == "HTTP/0.9" || http_ver == "HTTP/1.0") return (OLD_HTTP);
  if (http_ver != "HTTP/1.1") return (BAD_REQ);
  return (NO_ERROR);
}

/**
 * @brief recv()로 받은 클라이언트의 리퀘스트 메시지에서 entity_ 부분만 할당
 *
 * @param http 초기화할 대상
 * @param e  참조할 속성
 * @param client_msg recv()로 받은 클라이언트의 리퀘스트
 * @return t_error
 */
static t_error AllocEntity(t_http* http, t_elem* e, char* client_msg) {
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

    http->entity_ = new char[entity_len + 1];
    memcpy(http->entity_, client_msg + entity_start, entity_len);
    http->entity_[entity_len] = '\0';
    return (NO_ERROR);
  } catch (std::bad_alloc& e) {
    return (SYS_ERR);
  }
}

/**
 * @brief http->msg_ 참조해서 http->entity_에 동적 할당(오버라이딩)
 *
 * @param http http->msg_ 참조해서 http->entity_에 동적 할당
 * @param e 참조할 속성
 * @return t_error
 */
static t_error AllocEntity(t_http* http, t_elem* e) {
  ssize_t entity_len;
  try {
    entity_len = std::stol(http->header_["Content-Length"]);
    if (entity_len <= 0)
      return (BAD_REQ);
    else
      http->entity_length_ = entity_len;
  } catch (std::invalid_argument const& ex) {
    return (BAD_REQ);
  }
  try {
    const size_t entity_start = e->_header_crlf + DOUBLE_CRLF_LEN;

    http->entity_ = new char[entity_len + 1];
    memcpy(http->entity_, http->msg_.data() + entity_start, entity_len);
    http->entity_[entity_len] = '\0';
    return (NO_ERROR);
  } catch (std::bad_alloc& e) {
    return (SYS_ERR);
  }
}

/**
 * @brief http 요청의 헤더 부분 탐색하면서 http->header_에 매핑
 *        쿠키 헤더 존재하면 플래그 ON
 *
 * @param http
 * @param e
 * @return t_error
 */
t_error FillHeader(s_client_type* client_type, t_http* http, t_elem* e) {
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
      if (key == "Cookie") SetPrevCookie(client_type, http);
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

/**
 * @brief client stage를 method에 맞게 초기화
 *
 * @param client
 * @param http
 */
static void SetClientStage(s_client_type* client, t_http* http) {
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
 * @brief 파서에서 쓸 정보 초기화 및 에러 검출
 *
 * @param e    쓸 정보 담기
 * @param line recv()로 받은 메시지 : string
 * @return t_error
 */
t_error ElemInitializer(t_elem* e, std::string line) {
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

/**
 * @brief 파서에서 쓸 정보 초기화 및 에러 검출
 *
 * @param e
 * @param msg recv()로 받은 메시지 : vector<char>
 * @return t_error
 */
static t_error ElemInitializer(t_elem* e, std::vector<char> msg) {
  if (msg.empty()) {
    return (BAD_REQ);
  }

  std::vector<char>::iterator double_crlf_pos =
      FinderToVector(msg, DOUBLE_CRLF);
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

/**
 * @brief 이전 RequestHandler
 *
 * @param msg_len
 * @param udata
 * @param msg
 * @return t_error
 */
t_error RequestHandler(size_t msg_len, void* udata, char* msg) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);
  client_type->SetAccessTime();

  if (!msg) {
    return (RequestError(client_type, BAD_REQ, "message is null"));
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
    err_code = ElemInitializer(&e, line);
    if (err_code) {
      return (RequestError(client_type, err_code, "ElemInitializer()"));
    }
    err_code = InitLineParser(http, &e);
    if (err_code) {
      return (RequestError(client_type, err_code, "InitLineParser()"));
    }
    if (FillHeader(client_type, http, &e)) {
      return (RequestError(client_type, BAD_REQ, "FillHeader()"));
    }
    if (e._method == POST) {
      if ((err_code = AllocEntity(http, &e, msg))) {
        return (RequestError(client_type, err_code, "alloc()"));
      }
    }
    SetClientStage(client_type, http);
    err_code = convert_uri(
        http->init_line_["URI"],
        client_type->GetParentServer().GetServerConfig().location_configs_,
        *client_type);
  } catch (std::exception& e) {
    std::stringstream r;
    r << e.what();
    std::string msg = r.str();
    msg += " < RequestHandler() exception occur >";
    return (RequestError(client_type, BAD_REQ, msg));
  }
  return (err_code);
}

/**
 * @brief
 *
 * @param client_type
 * @param http
 * @return t_error
 */
static t_error PostHandler(s_client_type* client_type, t_http* http) {
  t_elem e;
  t_error err_code = NO_ERROR;
  try {
    err_code = ElemInitializer(&e, http->msg_);
    if (err_code)
      return (RequestError(client_type, err_code, "ElemInitializer()"));

    err_code = InitLineParser(http, &e);
    if (err_code)
      return (RequestError(client_type, err_code, "ElemInitializer()"));

    if (FillHeader(client_type, http, &e))
      return (RequestError(client_type, BAD_REQ, "FillHeader()"));

    err_code = AllocEntity(http, &e);
    if (err_code) return (RequestError(client_type, err_code, "AllocEntity()"));

  } catch (const std::exception& e) {
    return (BAD_REQ);
  }
  return (err_code);
}

static t_error GetHandler(s_client_type* client_type, t_http* http) {
  std::string line(http->msg_.begin(), http->msg_.end());
  t_elem e;
  t_error err_code = NO_ERROR;

  try {
    err_code = ElemInitializer(&e, line);
    if (err_code)
      return (RequestError(client_type, err_code, "ElemInitializer()"));

    err_code = InitLineParser(http, &e);
    if (err_code)
      return (RequestError(client_type, err_code, "ElemInitializer()"));

    if (FillHeader(client_type, http, &e))
      return (RequestError(client_type, BAD_REQ, "FillHeader()"));
  } catch (const std::exception& e) {
    return (BAD_REQ);
  }
  return (err_code);
}

t_error RequestHandler(void* udata) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  client_type->SetStage(REQ_READY);

  t_http* http = &client_type->GetRequest();
  t_error err_code = NO_ERROR;
  std::string method = http->init_line_["METHOD"];

  try {
    if (method == "POST")
      err_code = PostHandler(client_type, http);
    else if (method == "GET" || method == "DELETE")
      err_code = GetHandler(client_type, http);
    else
      return (RequestError(client_type, NOT_IMPLE, "구현하지 않은 메소드"));

    if (err_code)
      return (RequestError(client_type, err_code, "RequestHandler()"));

    err_code = convert_uri(
        http->init_line_["URI"],
        client_type->GetParentServer().GetServerConfig().location_configs_,
        *client_type);
    if (err_code)
      return (RequestError(client_type, err_code, "RequestHandler()"));

    SetClientStage(client_type, http);
  } catch (const std::exception& e) {
    return (RequestError(client_type, SYS_ERR, "rq_hdlr() 예외 발생"));
  }
  return (err_code);
}
