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
  std::string path = client->GetLocationConfig().location_;
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

static std::string to_hex_string(size_t value) {
  std::ostringstream os;
  unsigned char* p = reinterpret_cast<unsigned char*>(&value);
  for (size_t i = 0; i < sizeof(value); i++) {
    os << std::setfill('0') << std::setw(2) << std::hex
       << static_cast<unsigned>(*(p + i));
  }
  return os.str();
}

inline char* ChunkMsgFirst(char* send_size, std::string hex_size, t_http& res,
                           size_t chunk_size) {
  sprintf(send_size, "%s\r\n", hex_size.c_str());
  size_t len = sizeof(send_size);
  char* buf = new char[len + chunk_size + 2];
  std::memcpy(buf, send_size, len);
  std::memcpy(buf + len, res.entity_, chunk_size);
  std::memcpy(buf + len + chunk_size, "\r\n", 2);
  return buf;
}

inline char* ChunkMsgMain(char* send_size, std::string hex_size, t_http& res,
                          size_t chunk_size) {
  sprintf(send_size, "%s\r\n", hex_size.c_str());
  size_t len = sizeof(send_size);
  char* buf = new char[len + chunk_size + 2];
  std::memcpy(buf, send_size, len);
  std::memcpy(buf + len, res.entity_ + chunk_size, chunk_size);
  std::memcpy(buf + len + chunk_size, "\r\n", 2);
  return buf;
}

inline char* ChunkMsgEnd(char* send_size, std::string last_size, t_http& res,
                         size_t chunk_size, s_client_type* client) {
  sprintf(send_size, "%s\r\n", last_size.c_str());
  size_t len = sizeof(send_size);
  char* buf = new char[len + chunk_size + 4];
  std::memcpy(buf, send_size, len);
  std::memcpy(buf + len, res.entity_ + chunk_size, chunk_size);
  std::memcpy(buf + len + (chunk_size % res.entity_length_), "\r\n", 2);
  client->SetStage(RES_FIN);
  return buf;
}

static char* ChunkMsg(s_client_type* client, char* msg) {
  t_http res = client->GetResponse();
  size_t chunk_size = static_cast<size_t>(
      client->GetConfig().main_config_.find(BODY)->second.size());
  std::string hex_size = to_hex_string(chunk_size);
  std::string last_size = to_hex_string(chunk_size % res.entity_length_);
  char send_size[1024];

  if (client->GetChunkSize() == 0) {
    client->SetStage(RES_CHUNK);
    return msg;
  }
  if (client->GetStage() == RES_CHUNK) {
    if (client->GetChunkSize() == 0) {
      return (ChunkMsgFirst(send_size, hex_size, res, chunk_size));
    } else if (client->IncreaseChunked(chunk_size)) {
      return (ChunkMsgMain(send_size, hex_size, res, chunk_size));
    } else {
      return (ChunkMsgEnd(send_size, hex_size, res, chunk_size, client));
    }
  }
  sprintf(send_size, "0\r\n\r\n");
  char* buf = new char[5];
  std::memcpy(buf, send_size, 5);
  return buf;
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
  std::string cookie_id = client->GetCookieId();
  std::sprintf(buf, "my_cookie=%s; HttpOnly;", cookie_id.c_str());
  std::string set_cookie = std::string(buf);
  msg.header_.insert(std::make_pair(std::string("Set-Cookie :"), set_cookie));
  if (client->GetChunked()) {
    msg.header_.insert(
        std::make_pair(std::string("Content-Type :"), MakeContentType(client)));
    msg.header_.insert((std::make_pair(std::string("Transfer-Encoding :"),
                                       std::string("chunked"))));
    return (msg);
  }
  if (code == MOV_PERMAN) {
    msg.header_.insert(
        std::make_pair(std::string("location :"), std::string("uri")));
    msg.header_.insert(
        std::make_pair(std::string("Connection :"), std::string("Closed")));
    return (msg);
  }
  if (client->GetResponse().entity_) {
    msg.header_.insert(
        std::make_pair(std::string("Content-Type :"), MakeContentType(client)));
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
  if (client->GetChunked() || client->GetStage() == RES_CHUNK ||
      client->GetStage() == RES_FIN) {
    return (ChunkMsg(client, msg));
  }
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
