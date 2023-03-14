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
  t_http type = client->GetRequest();
  std::string path = type.init_line_.find("path")->second;
  const t_mime* mime = &client->GetConfig().mime_;
  std::string::size_type extension_pos = path.find_last_of(".");
  std::string extension = path.substr(extension_pos + 1);
  std::transform(extension.begin(), extension.end(), extension.begin(),
                 ::tolower);
  return (mime->at(extension));
}

static std::string stToString(size_t size) {
  size_t num = size;
  char buf[1024];
  std::sprintf(buf, "%lu", num);
  std::string str_num = std::string(buf);
  return str_num;
}

t_http MakeResponseMessages(s_client_type* client) {
  t_error code = client->GetErrorCode();
  t_http msg = client->GetResponse();
  std::string str_code = enumToString(code);
  std::time_t now = std::time(NULL);
  std::string date_str = std::asctime(std::gmtime(&now));
  date_str.erase(date_str.length() - 1);
  char buf[1024];

  msg.init_line_.insert(
      std::make_pair(std::string("version"), std::string("HTTP/1.1")));
  msg.init_line_.insert(std::make_pair(std::string("code"), str_code));
  msg.header_.insert(std::make_pair(std::string("Date :"), date_str));
  msg.header_.insert(
      std::make_pair(std::string("Server :"), std::string("serv1")));
  std::string cookie_id = stToString(client->GetCookieId());
  std::sprintf(buf, "my_cookie=%s; HttpOnly;", cookie_id);
  std::string set_cookie = std::string(buf);
  msg.header_.insert(std::make_pair(std::string("Set-Cookie :"), set_cookie));
  if (client->GetStage() == 301) {
    msg.header_.insert(
        std::make_pair(std::string("location :"), std::string("uri")));
    msg.header_.insert(
        std::make_pair(std::string("Connection :"), std::string("Closed")));
    return (msg);
  }
  if (client->GetResponse().entity_) {
    std::string size = stToString(client->GetResponse().entity_length_);
    msg.header_.insert(std::make_pair(std::string("Content-length :"), size));
  }
  msg.header_.insert(std::make_pair(std::string("Cache-Control"),
                                    std::string("public, max-age=3600")));
  if (client->GetStage() == END) {
    msg.header_.insert(
        std::make_pair(std::string("Connection :"), std::string("Closed")));
  } else {
    msg.header_.insert(
        std::make_pair(std::string("Connection :"), std::string("Keep-Alive")));
  }
  return (msg);
}

char* MaketopMessage(s_client_type* client) {
  t_http msg = client->GetResponse();
  std::string joined_str = "";
  std::string entity = msg.entity_;
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

  client->SetMessageLength(joined_str.length());
  char* result = new char[joined_str.length() + 1];
  std::strcpy(result, joined_str.c_str());
  return result;
}

char* MakeSendMessage(s_client_type* client, char* msg) {
  size_t len = client->GetMessageLength();
  size_t entity_length = client->GetResponse().entity_length_;

  char* result = new char[len + entity_length];
  std::memcpy(result, msg, len);
  std::memcpy(result + len, client->GetResponse().entity_, entity_length);
  client->SetMessageLength(len + entity_length);
  return (result);
}

void DeleteSendMessage(char* msg, size_t size) {
  for (std::size_t i = 0; i < size; ++i) {
    std::cout << msg[i] << " ";
  }
  delete[] msg;
}
