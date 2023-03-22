
#include "../../include/config.hpp"
#include "../../include/webserv.hpp"

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF_LEN 2
#define DOUBLE_CRLF_LEN 4
#define MAX_HEADER_SIZE 4000

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
  client.GetRequest().init_line_["URI"] = make_uri_path(rq_path);
  // print_vector_path(rq_path);
  return NO_ERROR;
}

inline void SetPrevCookie(s_client_type* client_type, t_http* http) {
  std::string cookie_line = http->header_["Cookie"];
  size_t equal_pos = cookie_line.find("=");
  if (equal_pos == std::string::npos) return;
  /* 발급 받은 Cookie가 없을 경우. (예: Cookie: id=) */
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
    std::cout << "msg size: " << http->msg_.size() << std::endl;
    std::cout << "entity len: " << http->entity_length_ << std::endl;
    if (http->msg_.size() == http->entity_length_)
      client->SetStage(POST_READY);
    else
      client->SetStage(REQ_ING);
  }
  else if (method == "DELETE")
    client->SetStage(DELETE_READY);
  return;
}

static int MethodParser(std::string s) {
  const std::string requisite[3] = {"GET", "POST", "DELETE"};

  for (int i = 0; i < 3; ++i) {
    if (s.find(requisite[i]) != std::string::npos)
      return (0);
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
    2. URI에 /delete?가 있으면, init_line_["METHOD"]와 e->_method를 DELETE로
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

t_error HeaderLineParser(s_client_type* client_type, t_http* http, std::string header_line) {
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

t_error EntityParser(t_http *http) {
  std::map<std::string, std::string>::iterator content_it = http->header_.find("Content-Length");
  std::map<std::string, std::string>::iterator chunked_it = http->header_.find("Transfer-encoding");

  if (content_it == http->header_.end() && chunked_it == http->header_.end())
    return (BAD_REQ);
  if ((*chunked_it).second == "chunked")
    return (NO_ERROR);

  std::string len = (*content_it).second;
  size_t entity_len;
  try {
    entity_len = std::stol(len);
    if (entity_len <= 0)
       return (BAD_REQ);
  } catch (std::invalid_argument const& ex) {
    return (BAD_REQ);
  }
  try {
    http->msg_.reserve(entity_len);
    http->entity_length_ = entity_len;
    http->temp_len_ = entity_len;
  } catch (const std::bad_alloc& e) {
    return (SYS_ERR);
  }
  return (NO_ERROR);
}

int RequestHandler(struct kevent* curr_event) {
  s_client_type* client_type = static_cast<s_client_type*>(curr_event->udata);
  t_http* http = &(client_type->GetRequest());
  char buf[MAX_HEADER_SIZE + 1];
  std::memset(buf, -1, MAX_HEADER_SIZE + 1);

  switch (client_type->GetStage()) {
    case DEF: {
      // -1 = 다 보냈을 때, 아직 읽을 준비가 안 됐을때, 읽을 것이 없을 때
      int result = recv(curr_event->ident, buf, MAX_HEADER_SIZE, 0);
      if (result == -1)
        return (-1);
      std::cout << "read size: " << result << std::endl;
      std::cout << "first msg size: " << http->msg_.size() << std::endl;
      buf[MAX_HEADER_SIZE] = '\0';
      // 4000만큼 읽었는데 헤더가 끝나지 않는 경우 -> BAD_REQ
      char* double_crlf = strstr(buf, DOUBLE_CRLF);

      std::ofstream hi("qqqq");
      hi <<  double_crlf;
      if (double_crlf == nullptr)
        return (RequestError(client_type, BAD_REQ, "DOUBLE CRLF가 없음"));

      std::string top(buf, double_crlf);

      const size_t first_crlf = top.find(CRLF);
      if (first_crlf == std::string::npos)
        return (RequestError(client_type, BAD_REQ, "CRLF가 없음"));

      std::string init_line = top.substr(0, first_crlf);
      std::string header = top.substr(first_crlf + CRLF_LEN, top.size() - (first_crlf + CRLF_LEN));
      t_error err_code = NO_ERROR;

      err_code = InitLineParser(http, init_line);
      if (err_code) return (RequestError(client_type, err_code, "RequestHandler.cpp/InitLineParser()"));

      err_code = HeaderLineParser(client_type, http, header);
      if (err_code) return (RequestError(client_type, err_code, "RequestHandler.cpp/HeaderLineParser()"));
      err_code = convert_uri(
        http->init_line_["URI"],
        client_type->GetParentServer().GetServerConfig().location_configs_,
        *client_type);
      if (err_code) return (RequestError(client_type, err_code, "RequestHandler.cpp/convert_uri()"));

      if (http->init_line_["METHOD"] == "POST") {
        err_code = EntityParser(http);
        if (err_code) return (RequestError(client_type, err_code, "RequestHandler.cpp/EntityParser()"));

        if (result > (double_crlf + DOUBLE_CRLF_LEN) - buf)
        {
          std::cout << "찌꺼기 넣기!!!!!\n";
          std::cout << "insert buf size: " << (buf + MAX_HEADER_SIZE) - (double_crlf + DOUBLE_CRLF_LEN) << std::endl;
          // http->msg_.insert(http->msg_.end(), double_crlf + DOUBLE_CRLF_LEN, buf + MAX_HEADER_SIZE);
          std::cout << "second result is " << result << std::endl;
          http->msg_.insert(http->msg_.end(), double_crlf + DOUBLE_CRLF_LEN, double_crlf + DOUBLE_CRLF_LEN + http->entity_length_);
          http->entity_ = http->msg_.begin().base();
          std::cout << "second msg size: " << http->msg_.size() << std::endl;
          if (http->msg_.size() == http->entity_length_)
            client_type->SetStage(POST_READY);
        }
        else
        {
          client_type->SetStage(REQ_ING);
        }
      }
      else
      {
        SetClientStage(client_type, http);
      }
    } break;
    case REQ_ING : {
      std::cout << "req_ing in " << std::endl;
      int result = recv(curr_event->ident, buf, MAX_HEADER_SIZE, 0);
      if (result == -1)
        return (-1);
      http->temp_len_ -= result;
      http->msg_.insert(http->msg_.end(), buf, buf + result);
      std::cout << "ING!!!" << std::endl;
      if (http->temp_len_ <= 0) {
        http->entity_ = strstr(http->msg_.begin().base(), DOUBLE_CRLF) + DOUBLE_CRLF_LEN;
        int index = 0;
        while (1)
        {
          if (http->entity_[index] == '-')
          {
            if (http->entity_[index + 1] == '-')
            {
              if (http->entity_[index + 2] == '-')
                break;
            }
          }
          index++;
        }
        http->entity_length_ = index;
        std::cout << "len " << http->entity_length_ << std::endl;
        client_type->SetStage(POST_READY);
      }
      }  break;

    default:
      break;
  }
  return (0);
}
