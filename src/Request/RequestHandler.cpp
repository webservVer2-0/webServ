#include "../../include/config.hpp"
#include "../../include/webserv.hpp"

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF_LEN 2
#define DOUBLE_CRLF_LEN 4

inline void show_map(std::map<std::string, std::string> myMap) {
  for (std::map<std::string, std::string>::iterator it = myMap.begin();
       it != myMap.end(); ++it) {
    std::cout << it->first << " : " << it->second << std::endl;
  }
}

inline void show_vec(std::vector<char> vec) {
  int i = 0;
  for (std::vector<char>::iterator it = vec.begin(); it != vec.end(); ++it) {
    std::cout << i << ": " << *it << std::endl;
    i++;
  }
  std::cout << std::endl;
}

inline long StringToLong(std::string str) {
  char* end;
  const int decimal = 10;
  long num = std::strtol(str.c_str(), &end, decimal);
  if (end == str.c_str() + str.length())
    return (num);
  else
    return (0);
}

inline long StringToHexLong(std::string str) {
  char* end;
  const int hex = 16;
  long num = std::strtol(str.c_str(), &end, hex);
  if (num < 0) return (-1);
  if (end == str.c_str() + str.length())
    return (num);
  else
    return (-1);
}

static t_error RefineChunk(t_http* http, size_t entity_len, int max_body_size) {
  char* entity = http->entity_;
  std::vector<char> temp_entity;
  std::string chunk_size;

  if (strnstr(entity, "0\r\n\r\n", entity_len) == NULL) {
    return (BAD_REQ);
  }
  int i = 0;
  int total_chunk = 0;
  // curl이 total chunk size를 넣어서 보내줌, 그 부분을 건너뛰기
  while (entity[i] != '\r' && entity[i + 1] != '\n') {
    i++;
    total_chunk++;
  }
  i += CRLF_LEN;
  int size = 0;
  bool is_chunk_size = true;  // chunk size를 보내는 라인 여부
  while (!(entity[i] == '0' && entity[i + 1] == '\r' && entity[i + 2] == '\n' &&
           entity[i + 3] == '\r' && entity[i + 4] == '\n')) {
    if (is_chunk_size) {
      // 16 진수인지 확인(0~9, a~f, A~F)
      if (isxdigit(entity[i])) {
        chunk_size.push_back(entity[i]);
      }
      // chunk size 끝
      else if (entity[i] == '\r' && entity[i + 1] == '\n') {
        // string을 long으로 변환
        size = StringToHexLong(chunk_size);
        if (size == -1 || size > max_body_size) {
          return (BAD_REQ);
        }
        is_chunk_size = false;
        chunk_size.clear();
      }
      // 16진수가 아니면
      else {
        return (BAD_REQ);
      }
    } else {
      // CRLF가 아닌 경우
      if (entity[i] == '\r' && entity[i + 1] == '\n') {
        if (size != -1) {
          return (BAD_REQ);
        }
        i += 1;  // CRLF 스킵
      } else {
        if (entity[i] != '\n' || (entity[i] == '\n' && entity[i - 1] == '\n'))
          temp_entity.push_back(entity[i]);
        else if (entity[i] == '\n' && entity[i + 1] == '\r')
          temp_entity.push_back(entity[i]);
        size--;
        if (size == -1) {
          is_chunk_size = true;
          i += CRLF_LEN;
        }
      }
    }
    i++;
  }
  // show_vec(temp_entity);
  delete[] http->entity_;
  http->entity_ = NULL;
  http->entity_ = new char[i - total_chunk - CRLF_LEN];
  http->entity_length_ = i - total_chunk - CRLF_LEN;
  for (size_t i = 0; i < temp_entity.size(); i++) {
    entity[i] = temp_entity[i];
  }
  http->entity_length_ = temp_entity.size();
  return (NO_ERROR);
}

std::string MakeUriPath(std::vector<std::string>& vec) {
  std::string ret;

  ret.append(".");
  for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();
       ++it) {
    ret.append("/" + *it);
  }
  return ret;
}

t_error ConvertUri(std::string rq_uri,
                   std::map<std::string, t_loc*> location_config,
                   s_client_type& client) {
  std::vector<std::string> rq_path;
  size_t pos;
  std::map<std::string, t_loc*>::iterator loc_it;
  size_t query_index;

  if ((query_index = rq_uri.find('?')) != std::string::npos) {
    t_http& http = client.GetRequest();
    const std::string& query = rq_uri.substr(query_index + 1);
    http.entity_length_ = query.length();
    // http.temp_entity_.clear();
    http.temp_entity_.reserve(http.entity_length_);
    http.temp_entity_.insert(http.temp_entity_.end(), query.begin(),
                             query.end());
    http.entity_ = new char[http.temp_entity_.size()];
    for (size_t i = 0; i < http.temp_entity_.size(); i++) {
      http.entity_[i] = http.temp_entity_[i];
    }
    rq_uri = rq_uri.substr(0, query_index);
  }
  std::string token = rq_uri;
  while ((pos = rq_uri.find('/')) != std::string::npos) {
    token = rq_uri.substr(0, pos);
    if (token != "" && token != ".") rq_path.push_back(token);
    rq_uri.erase(0, pos + 1);
  }
  if (rq_uri != ".") rq_path.push_back(rq_uri);
  loc_it = location_config.find(("/" + rq_path.front()));
  if (loc_it == location_config.end()) {
    loc_it = location_config.find("/");
    rq_path.insert(rq_path.begin(), (*loc_it).second->main_config_[ROOT]);
  } else {
    rq_path.front() = (*loc_it).second->main_config_[ROOT];
  }
  if (rq_path.size() == 1)
    rq_path.push_back((*loc_it).second->main_config_[DEFFILE]);

  if ((*loc_it).second->main_config_[METHOD].find(
          client.GetRequest().init_line_["METHOD"]) == std::string().npos) {
    return FORBID;
  }
  client.SetConfigPtr((*loc_it).second);
  client.GetRequest().init_line_["URI"] = MakeUriPath(rq_path);
  return NO_ERROR;
}

inline void SetPrevCookie(s_client_type* client_type, t_http* http) {
  std::string cookie_line = http->header_["Cookie"];
  size_t equal_pos = cookie_line.find("=");
  if (equal_pos == std::string::npos) return;

  /* 발급 받은 Cookie가 없을 경우. (예: Cookie: id=) */
  if (equal_pos + 1 == cookie_line.size()) return;
  size_t semicolon_pos = cookie_line.find(";");
  std::string prev_id =
      cookie_line.substr(equal_pos + 1, semicolon_pos - equal_pos - 1);
  int timer = atoi(client_type->GetConfig()
                       .main_config_.find(TIMEOUT)
                       .
                       operator->()
                       ->second.c_str());
  if (timer != 0) {
    ServerConfig::ChangeEvents(client_type->GetFD(), EVFILT_TIMER, EV_CLEAR,
                               NOTE_SECONDS, timer, client_type);
  }
  for (size_t i = 0; i < prev_id.size(); i++) {
    if (!isnumber(prev_id[i])) return;
  }
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
static int RequestError(s_client_type* client_type, t_error err_code,
                        std::string msg) {
  int errno_ = errno;
  client_type->SetErrorString(errno_, msg);
  client_type->SetErrorCode(err_code);
  client_type->SetStage(ERR_READY);
  return (1);
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
  else if (method == "POST") {
    if (http->temp_entity_.size() == http->entity_length_)
      client->SetStage(POST_READY);
    else
      client->SetStage(REQ_ING);
  } else if (method == "DELETE")
    client->SetStage(DELETE_READY);
  return;
}

static int MethodParser(std::string s) {
  const std::string requisite[3] = {"GET", "POST", "DELETE"};

  for (int i = 0; i < 3; ++i) {
    if (s.find(requisite[i]) != std::string::npos) return (0);
  }
  return (1);
}

t_error InitLineParser(t_http* http, std::string line) {
  if (MethodParser(line)) return (NOT_IMPLE);

  size_t first_space = line.find(" ");
  if (first_space == std::string::npos) return (BAD_REQ);

  try {
    http->init_line_["METHOD"] = line.substr(0, first_space);
  } catch (std::exception) {
    return (SYS_ERR);
  }

  size_t second_space = line.find(" ", first_space + 1);
  if (second_space == std::string::npos) return (BAD_REQ);
  /*
    1. t_http의 init_line_["URI"] 초기화
    2. URI에 /delete?가 있으면, init_line_["METHOD"]와 e->_method를
    DELETE로
  */
  try {
    http->init_line_["URI"] =
        line.substr(first_space + 1, second_space - first_space - 1);
    /* TODO: delete일때 작동 확인 */
    if (http->init_line_["METHOD"] == "GET" &&
        http->init_line_["URI"].find("/delete?") != std::string::npos) {
      http->init_line_["METHOD"] = "DELETE";
    }
  } catch (std::exception) {
    return (SYS_ERR);
  }
  std::string http_ver;
  try {
    http_ver = line.substr(second_space + 1, line.size() - second_space + 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  if (http_ver == "HTTP/0.9" || http_ver == "HTTP/1.0") return (OLD_HTTP);
  if (http_ver != "HTTP/1.1") return (BAD_REQ);
  return (NO_ERROR);
}

t_error HeaderLineParser(s_client_type* client_type, t_http* http,
                         std::string header_line) {
  size_t pos = 0;
  size_t end_pos;
  std::string key;
  std::string value;

  while ((end_pos = header_line.find(CRLF, pos)) != std::string::npos) {
    std::string line = header_line.substr(pos, end_pos - pos);
    size_t colon_pos = line.find(": ");

    if (colon_pos != std::string::npos) {
      key = line.substr(0, colon_pos);
      value = line.substr(colon_pos + CRLF_LEN);
      http->header_[key] = value;
      if (key.find("Cookie") == 0) {
        SetPrevCookie(client_type, http);
      }
    } else if (pos == end_pos) {
      break;
    } else {
      return (BAD_REQ);
    }
    pos = end_pos + CRLF_LEN;
  }
  return (NO_ERROR);
}

t_error EntityParser(t_http* http, s_client_type* client) {
  std::map<std::string, std::string>::iterator content_it =
      http->header_.find("Content-Length");
  std::map<std::string, std::string>::iterator chunked_it =
      http->header_.find("Transfer-Encoding");

  if (content_it == http->header_.end() && chunked_it == http->header_.end())
    return (BAD_REQ);
  if ((*chunked_it).second == "chunked") {
    if (http->header_["User-Agent"].find("curl/") == std::string::npos) {
      return (NOT_IMPLE);
    }
    return (NO_ERROR);
  }
  if (content_it != http->header_.end()) {
    if (content_it == http->header_.end()) return (BAD_REQ);
    int entity_len = atoi(content_it.operator->()->second.c_str());
    if (entity_len <= 0) return (BAD_REQ);
    int max_len = atoi(client->GetConfig()
                           .main_config_.find(BODY)
                           .
                           operator->()
                           ->second.c_str());
    if (entity_len > max_len) return (BAD_REQ);
  }

  std::string len = (*content_it).second;
  const long entity_len = StringToLong(len);
  if (entity_len <= 0) return (BAD_REQ);

  try {
    http->temp_entity_.clear();
    http->temp_entity_.reserve(entity_len);

    http->entity_length_ = entity_len;
    http->content_len_ = entity_len;
  } catch (const std::bad_alloc& e) {
    return (SYS_ERR);
  }
  return (NO_ERROR);
}

/**
 * @brief buf를 붙인 벡터에서 boundary를 제외하고 entity만 분리하는 함수
 *
 * @param client_type
 * @param http
 * @return int 에러가 있으면 1 return, 없으면 0 return
 */
inline int RefineEntity(s_client_type* client_type, t_http* http) {
  char* double_crlf_temp_entity = NULL;
  size_t j = 0;
  size_t k = 0;

  for (size_t i = 0; i < http->temp_entity_.size(); ++i) {
    if (http->temp_entity_[i] == '\r' && http->temp_entity_[i + 1] == '\n' &&
        http->temp_entity_[i + 2] == '\r' &&
        http->temp_entity_[i + 3] == '\n') {
      double_crlf_temp_entity = new char[http->temp_entity_.size() - (i + 3)];
      j = i + 4;
      while (j < http->temp_entity_.size()) {
        double_crlf_temp_entity[k] = http->temp_entity_[j];
        j++;
        k++;
      }
      break;
    }
  }
  if (double_crlf_temp_entity == NULL) {
    return (0);
  }

  http->entity_ = double_crlf_temp_entity;
  http->entity_length_ = k;

  // 크롬/사파리의 기준 key값
  // TODO : 크롬/사파리 외에도 테스트 필요
  const std::string boundary = "boundary=----WebKitFormBoundary";
  // boundary=-- 이후의 문자열을 key로 사용
  std::string key = http->header_["Content-Type"].substr(
      http->header_["Content-Type"].find(boundary) + boundary.size());
  char* found = NULL;
  for (size_t i = 0; i < http->entity_length_ - key.size() + 1; ++i) {
    if (std::memcmp(&http->entity_[i], key.c_str(), key.size()) == 0) {
      found = &http->entity_[i];
      break;
    }
  }
  // entity에서 key를 찾지 못했을 경우 (key로 끝나지 않을 경우)
  if (found == NULL) {
    return (RequestError(client_type, BAD_REQ,
                         "RequestHandler.cpp/RefineEntity() Error"));
  }
  http->entity_length_ = found - http->entity_ - 5;
  return (0);
}

int RequestHandler(struct kevent* curr_event) {
  s_client_type* client_type = static_cast<s_client_type*>(curr_event->udata);
  t_http* http = &(client_type->GetRequest());
  long max_header_size = StringToLong(
      client_type->GetParentServer().GetServerConfig().main_config_[MAXH]);
  int max_body_size = StringToLong(
      client_type->GetParentServer().GetServerConfig().main_config_[BODY]);
  // char buf[max_header_size + 1];
  static char* buf = new char[max_header_size + 1];
  std::memset(buf, -1, max_header_size + 1);

  switch (client_type->GetStage()) {
    case DEF: {
      // -1 = 다 보냈을 때, 아직 읽을 준비가 안 됐을때, 읽을 것이 없을 때
      int read_byte = recv(curr_event->ident, buf, max_header_size, 0);
      if (read_byte == -1) return (-1);
      // 4000만큼 읽었는데 헤더가 끝나지 않는 경우 -> BAD_REQ
      char* double_crlf = strnstr(buf, DOUBLE_CRLF, read_byte);

      if (double_crlf == NULL)
        return (RequestError(client_type, BAD_REQ, "DOUBLE CRLF가 없음"));

      std::string top(buf, double_crlf + DOUBLE_CRLF_LEN);

      const size_t first_crlf = top.find(CRLF);
      if (first_crlf == std::string::npos)
        return (RequestError(client_type, BAD_REQ, "CRLF가 없음"));

      std::string init_line = top.substr(0, first_crlf);
      std::string header = top.substr(first_crlf + CRLF_LEN,
                                      top.size() - (first_crlf + CRLF_LEN));
      t_error err_code = NO_ERROR;

      err_code = InitLineParser(http, init_line);
      if (err_code)
        return (RequestError(client_type, err_code,
                             "RequestHandler.cpp/InitLineParser()"));

      err_code = HeaderLineParser(client_type, http, header);
      if (err_code)
        return (RequestError(client_type, err_code,
                             "RequestHandler.cpp/HeaderLineParser()"));

      client_type->SetOriginURI(
          http->init_line_.find("URI").operator->()->second);

      err_code = ConvertUri(
          http->init_line_["URI"],
          client_type->GetParentServer().GetServerConfig().location_configs_,
          *client_type);
      if (err_code)
        return (RequestError(client_type, err_code,
                             "RequestHandler.cpp/ConvertUri()"));

      if (http->init_line_["METHOD"] == "POST") {
        err_code = EntityParser(http, client_type);
        if (err_code)
          return (RequestError(client_type, err_code,
                               "RequestHandler.cpp/EntityParser()"));

        // 처음 읽은 buf에서 entity까지 있는 경우
        const bool entity_exist =
            (double_crlf + DOUBLE_CRLF_LEN) - buf < read_byte;
        if (entity_exist) {
          // entity가 있을 때, buf에 남아있는 entity를 temp_entity_에
          // 넣어둔다.
          http->temp_entity_.insert(
              http->temp_entity_.end(), double_crlf + DOUBLE_CRLF_LEN,
              double_crlf + DOUBLE_CRLF_LEN +
                  (read_byte - (double_crlf + DOUBLE_CRLF_LEN - buf)));

          // entity_에 temp_entity_의 시작 주소 대입
          http->entity_ = new char[http->temp_entity_.size()];
          for (size_t i = 0; i < http->temp_entity_.size(); i++) {
            http->entity_[i] = http->temp_entity_[i];
          }

          if (http->header_["Transfer-Encoding"].find("chunked") !=
              std::string::npos) {
            err_code = RefineChunk(
                http, read_byte - (double_crlf + DOUBLE_CRLF_LEN - buf),
                max_body_size);
            if (err_code) {
              return (RequestError(client_type, err_code,
                                   "RequestHandler.cpp/RefineChunk()"));
            }
            client_type->SetStage(POST_READY);
            return (0);
          }

          // content_length_와 temp_entity_의 크기가 같으면
          // ENTITY를 가공할 필요 없으니 바로 POST_READY
          if (http->temp_entity_.size() == http->entity_length_) {
            client_type->SetStage(POST_READY);
          } else {
            client_type->SetStage(REQ_ING);
          }
        } else {
          client_type->SetStage(REQ_ING);
        }
      }
      SetClientStage(client_type, http);
    } break;
    case REQ_ING: {
      int read_byte = recv(curr_event->ident, buf, max_header_size, 0);
      if (read_byte == -1) return (-1);

      // 읽은만큼 content_len_에서 빼줌
      http->content_len_ -= read_byte;

      // 벡터 뒤에 buf를 붙임
      http->temp_entity_.insert(http->temp_entity_.end(), buf, buf + read_byte);

      // 다 읽었으면 'boundary=----WebKitFormBoundary' 떼어주고
      // POST_READY로
      if (http->content_len_ <= 0) {
        if (RefineEntity(client_type, http)) {
          return (RequestError(client_type, BAD_REQ,
                               "RequestHandler.cpp/RefineEntity()"));
        }
        client_type->SetStage(POST_READY);
      }
    } break;

    default:
      break;
  }
  return (0);
}
