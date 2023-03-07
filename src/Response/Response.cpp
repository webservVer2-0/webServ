#include <stdio.h>
#include <stdlib.h>

#include "../../include/utils.hpp"

std::string enumToString(t_error code) {
  switch (code) {
    case NO_ERROR:
      return "0 NO_ERROR";
    case OK:
      return "200 OK";
    case BAD_REQ:
      return "400 BAD_REQ";
    case FORBID:
      return "403 FORBID";
    case NOT_FOUND:
      return "404 NOT_FOUND";
    case NOT_IMPLE:
      return "501 NOT_IMPLE";
    case OLD_HTTP:
      return "505 OLD_HTTP";
    case SYS_ERR:
      return "500 SYS_ERR";
    default:
      return "UNKNOWN_ERROR";
  }
}

std::string MakeContentType(s_client_type* client) {
  t_html type = client->GetRequest();
  std::string path = type.init_line_.find("path")->second;

  std::map<std::string, std::string> mime_types = {{"txt", "text/plain"},
                                                   {"html", "text/html"},
                                                   {"jpeg", "image/jpeg"},
                                                   {"png", "image/png"},
                                                   {"py", "text/x-python"}};
  std::string::size_type extension_pos = path.find_last_of(".");
  if (extension_pos == std::string::npos) {
    std::cerr << "Cannot find extension in path." << std::endl;
    return ("");
    // error message 고려 사항
  }

  std::string extension = path.substr(extension_pos + 1);
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 ::tolower);
  std::map<std::string, std::string>::const_iterator iter =
      mime_types.find(extension);
  if (iter == mime_types.end()) {
    std::cerr << "Cannot find available MIME_Type." << std::endl;
    return ("");
    // error message 고려 사항
  }
  std::string content_type = iter->second;
  return (content_type);
}

std::string stToString(size_t size) {
  size_t num = size;
  char buf[1024];
  std::sprintf(buf, "%lu", num);
  std::string str_num = std::string(buf);
}

t_html MakeResponseMessages(s_client_type* client) {
  t_error code = client->GetErrorCode();
  t_html msg = client->GetResponse();
  std::string str_code = enumToString(code);
  std::time_t now = std::time(NULL);
  std::string date_str = std::asctime(std::gmtime(&now));
  date_str.erase(date_str.length() - 1);

  msg.init_line_.insert({"a_version", "HTTP/1.1"});
  msg.init_line_.insert({"code", str_code});
  msg.header_.insert({"Date :", date_str});
  msg.header_.insert({"Server :", "webserv/0.1"});
  if (client->GetResponse().entity) {
    std::string size = stToString(client->GetResponse().entity_length_);
    msg.header_.insert({"Content-Length :", size});
    std::string header_str = MakeContentType(client);
    msg.header_.insert({"Content-Type :", header_str});
  }
  if (client->GetStage() == END) {
    msg.header_.insert({"Connection :", "Closed"});
  }
  msg.header_.insert({"Connection :", "Keep-Alive"});
  return (msg);
  // TODO: Expire 시간 논의 필요(last-modified와 연계 고려 가능)
  // TODO: Cache-Control 사용 여부 확인 필요
  // TODO: last-modified 필요

  /*  switch (code) {
      case OK:
        std::cout << "OK_stage" << std::endl;
        {
          // msg.header_.insert({"Last-Modified :", date_str});
          // 본문 마지막 수정 시간 정보 필요
          msg.header_.insert({"Connection :", "close"});
        }
        break;
      case BAD_REQ:
        std::cout << "BAD_REQ_stage" << std::endl;
        {}
        break;
      case FORBID:
        std::cout << "FORBID_stage" << std::endl;
        {}
        break;
      case NOT_FOUND:
        std::cout << "NOT_FOUND_stage" << std::endl;
        {}
        break;
      case NOT_IMPLE:
        std::cout << "NOT_IMPLE_stage" << std::endl;
        {}
        break;
      case OLD_HTTP:
        std::cout << "OLD_HTTP_stage" << std::endl;
        {}
        break;
      default:  // SYS_ERR
        std::cout << "SYS_ERR_stage" << std::endl;
        {}
        break;
    }*/
}

char* MakeSendMessage(s_client_type client) {
  t_html msg = client.GetResponse();
  std::string joined_str = "";
  std::string entity = msg.entity;
  for (std::map<std::string, std::string>::const_iterator iter =
           msg.init_line_.begin();
       iter != msg.init_line_.end(); ++iter) {
    joined_str += iter->second + " ";
  }
  joined_str += "\r\n";
  for (std::map<std::string, std::string>::const_iterator iter =
           msg.header_.begin();
       iter != msg.header_.end(); ++iter) {
    joined_str += iter->first + iter->second + "\r\n";
  }
  joined_str += "\r\n\r\n";

  client.SetMessageLength(joined_str.length());
  char* result = new char[joined_str.length() + 1];
  std::strcpy(result, joined_str.c_str());
  return result;
}
