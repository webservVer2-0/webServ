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

static std::string stToString(size_t size) {
  size_t num = size;
  char buf[1024];
  std::snprintf(buf, 1024, "%lu", num);
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
  snprintf(send_size, 1024, "%s\r\n", hex_size.c_str());
  size_t len = sizeof(send_size);
  char* buf = new char[len + chunk_size + 2];
  std::memcpy(buf, send_size, len);
  std::memcpy(buf + len, res.entity_, chunk_size);
  std::memcpy(buf + len + chunk_size, "\r\n", 2);
  return buf;
}

inline char* ChunkMsgMain(char* send_size, std::string hex_size, t_http& res,
                          size_t chunk_size) {
  snprintf(send_size, 1024, "%s\r\n", hex_size.c_str());
  size_t len = sizeof(send_size);
  char* buf = new char[len + chunk_size + 2];
  std::memcpy(buf, send_size, len);
  std::memcpy(buf + len, res.entity_ + chunk_size, chunk_size);
  std::memcpy(buf + len + chunk_size, "\r\n", 2);
  return buf;
}

inline char* ChunkMsgEnd(char* send_size, std::string last_size, t_http& res,
                         size_t chunk_size, s_client_type* client) {
  snprintf(send_size, 1024, "%s\r\n", last_size.c_str());
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
  std::string chunk = static_cast<std::string>(
      client->GetConfig().main_config_.find(BODY)->second.data());
  size_t chunk_size = std::atoi(chunk.c_str());
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
  snprintf(send_size, 1024, "0\r\n\r\n");
  char* buf = new char[5];
  std::memcpy(buf, send_size, 5);
  return buf;
}

inline t_http MakeResInitHeader(s_client_type* client, t_http msg,
                                std::string str_code, std::string date_str,
                                char* buf) {
  msg.init_line_.insert(
      std::make_pair(std::string("version"), std::string("HTTP/1.1 ")));
  msg.init_line_.insert(std::make_pair(std::string("code"), str_code));
  msg.header_.insert(std::make_pair(std::string("Date: "), date_str));
  msg.header_.insert(
      std::make_pair(std::string("Server: "),
                     client->GetConfig().main_config_.at("server_name")));
  std::string cookie_id = client->GetCookieId();
  std::cout << "cookie_id : " << cookie_id << std::endl;
  std::snprintf(buf, 1024, "id=%s; HttpOnly;", cookie_id.c_str());
  std::string set_cookie = std::string(buf);
  msg.header_.insert(std::make_pair(std::string("Set-Cookie: "), set_cookie));
  return (msg);
}

inline t_http MakeChunkHeader(s_client_type* client, t_http msg) {
  msg.header_.insert(
      std::make_pair(std::string("Content-Type: "), client->GetMimeType()));
  msg.header_.insert((std::make_pair(std::string("Transfer-Encoding: "),
                                     std::string("chunked"))));
  return (msg);
}

inline t_http MakePermanHeader(s_client_type* client, t_http msg) {
  msg.header_.insert(std::make_pair(
      std::string("Location: "),
      client->GetLocationConfig().main_config_.at("redirection")));
  msg.header_.insert(
      std::make_pair(std::string("Connection: "), std::string("Closed")));
  return (msg);
}

inline t_http MakeEntityHeader(s_client_type* client, t_http msg) {
  msg.header_.insert(
      std::make_pair(std::string("Content-Type: "), client->GetMimeType()));
  std::string size = stToString(client->GetResponse().entity_length_);
  msg.header_.insert(std::make_pair(std::string("Content-Length: "), size));

  msg.header_.insert(std::make_pair(std::string("Cache-Control: "),
                                    std::string("public, max-age=3600")));
  return (msg);
}

t_http MakeResponseMessages(s_client_type* client) {
  t_error code = client->GetErrorCode();
  t_http msg = client->GetResponse();
  std::string str_code = enumToString(code);
  std::time_t now = std::time(NULL);
  std::string date_str = std::asctime(std::gmtime(&now));
  date_str.erase(date_str.length() - 1);
  char buf[1024];

  msg = MakeResInitHeader(client, msg, str_code, date_str, buf);

  if (client->GetChunked()) {
    msg = MakeChunkHeader(client, msg);
    return (msg);
  }
  if (code == MOV_PERMAN) {
    msg = MakePermanHeader(client, msg);
    return (msg);
  }
  if (client->GetResponse().entity_) {
    msg = MakeEntityHeader(client, msg);
  }
  if (client->GetStage() == END) {
    msg.header_.insert(
        std::make_pair(std::string("Connection: "), std::string("Closed")));
  } else {
    msg.header_.insert(
        std::make_pair(std::string("Connection: "), std::string("Keep-Alive")));
  }
  return (msg);
}

char* MaketopMessage(s_client_type* client) {
  t_http msg = client->GetResponse();
  std::string joined_str = "";
  joined_str.append(msg.init_line_.at("version"));
  joined_str.append(msg.init_line_.at("code"));
  joined_str += "\r\n";
  for (std::map<std::string, std::string>::const_iterator iter =
           msg.header_.begin();
       iter != msg.header_.end(); ++iter) {
    joined_str += iter->first + iter->second + "\r\n";
  }
  joined_str += "\r\n";
  client->SetMessageLength(joined_str.length());
  char* result = new char[joined_str.length()];
  std::memcpy(result, joined_str.c_str(), joined_str.length());
  return result;
}

char* MakeSendMessage(s_client_type* client, char* msg) {
  if (client->GetChunked() || client->GetStage() == RES_FIN) {
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

void SendMessageLength(s_client_type* client) {
  size_t send_msg_len;

  if (client->GetStage() == RES_CHUNK) {
    send_msg_len = static_cast<size_t>(
        client->GetConfig().main_config_.find(BODY)->second.size());
  } else if (client->GetStage() == RES_CHUNK &&
             client->GetChunkSize() > client->GetResponse().entity_length_) {
    send_msg_len =
        client->GetChunkSize() % client->GetResponse().entity_length_;
  } else if (client->GetStage() == RES_FIN) {
    send_msg_len = 5;
  } else
    send_msg_len = client->GetMessageLength();
  client->SetMessageLength(send_msg_len);
}

static void SendChunk(struct kevent* event, s_client_type* client) {
  if (client->GetSendNum() == DEF) {
    client->SetSendNum(client->GetStage());
  }
  size_t len = client->GetMessageLength();
  size_t send_len = client->GetSendLength();
  size_t temp_len =
      send(event->ident, client->GetBuf() + send_len, len - send_len, 0);
  if (temp_len < 0) {
    client->SetStage(RES_SEND);
  } else if (temp_len < len) {
    client->SetStage(RES_SEND);
    temp_len += send_len;
    client->SetSendLength(temp_len);
  } else {
    delete const_cast<char*>(client->GetBuf());
    client->SetBuf(NULL);
    client->SetSendLength(0);
    client->SetStage(client->GetSendNum());
  }
}
void SendProcess(struct kevent* event, s_client_type* client) {
  if (client->GetChunked() || client->GetStage() == RES_FIN) {
    SendChunk(event, client);
  }
  size_t len = client->GetMessageLength();
  size_t send_len = client->GetSendLength();
  size_t temp_len =
      send(event->ident, client->GetBuf() + send_len, len - send_len, 0);
  if (temp_len < 0) {
    client->SetStage(RES_SEND);
  } else if (temp_len < len) {
    client->SetStage(RES_SEND);
    temp_len += send_len;
    client->SetSendLength(temp_len);
  } else {
    delete const_cast<char*>(client->GetBuf());
    client->SetBuf(NULL);
    client->SetSendLength(0);
    client->SetMessageLength(0);
    client->SetStage(DEF);
  }
}

void SendFin(struct kevent* event, s_client_type* client) {
  client->SendLogs();
  if (client->GetStage() == END) {
    ServerConfig::ChangeEvents(event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, 0);
    close(event->ident);
    ResetConnection(static_cast<s_client_type*>(event->udata));
  } else {
    ServerConfig::ChangeEvents(event->ident, EVFILT_WRITE, EV_DISABLE, 0, 0, 0);
    ServerConfig::ChangeEvents(event->ident, EVFILT_READ, EV_ENABLE, 0, 0,
                               client);
    ResetConnection(static_cast<s_client_type*>(event->udata));
  }
}
