#include "../../include/utils.hpp"

#include <stdio.h>
#include <stdlib.h>

void PrintError(int size, const char* first, ...) {
  char* val = NULL;
  va_list list;

  va_start(list, first);

  SOUT << RED;
  SOUT << first << " : ";
  for (int i = 1; i < size; i++) {
    val = va_arg(list, char*);
    SOUT << val;
    if (i + 1 != size) {
      SOUT << " : ";
    }
  }

  SOUT << RESET << SEND;
  va_end(list);
  exit(1);
}

bool IsExistFile(const char* file_path) {
  int ret;

  ret = access(file_path, R_OK | F_OK);
  if (ret == -1) {
    return (false);
  }
  return (true);
}

int GetFileSize(const char* file_path) {
  struct stat file_info;
  int ret;

  ret = stat(file_path, &file_info);
  if (ret < 0) {
    return (-1);
  }
  return (file_info.st_size);
}

char* GetFile(const char* file_path) {
  int fd = 0;
  int size = GetFileSize(file_path);
  char* storage;
  storage = new char[size + 1];
  if (storage == NULL) {
    PrintError(3, WEBSERV, CRITICAL, "HEAP ASSIGNMENT");
  }
  fd = open(file_path, O_RDONLY);
  int ret = read(fd, storage, size + 1);
  if (ret == -1) {
    PrintError(3, WEBSERV, CRITICAL, "IO READING FAIL");
  }
  close(fd);
  storage[size] = '\0';
  return (storage);
}

bool IsWhiteSpace(char ptr) {
  return ptr == ' ' || ptr == '\t' || ptr == '\n' || ptr == '\r';
}

void PrintLine(std::string& target, pos_t pos) {
  SOUT << "Target Size : " << target.size() << SEND;
  SOUT << "Target Starting Point : " << pos << SEND;
  while (target.at(pos) != '\n') {
    SOUT << target.at(pos);
    pos++;
  }
  SOUT << SEND;
}

pos_t FindKeyLength(std::string& str, pos_t& pos) {
  pos_t ret = 0;
  const char* target = str.c_str();
  while (!IsWhiteSpace(str[pos])) {
    if (isalpha(target[pos]) || (target[pos] == '_') || target[pos] == '/' ||
        target[pos] == '.') {
      pos++;
      ret++;
    } else {
      PrintError(4, WEBSERV, CRITICAL, "NOT SUPPORT CONFIG KEY",
                 str.substr(pos, 1).c_str());
    }
  }
  return (ret);
}

pos_t FindValueLength(std::string& str, pos_t& pos) {
  pos_t ret = 0;
  while (IsWhiteSpace(str[pos])) {
    pos++;
  }
  pos_t i = pos;
  while (str[i] != ';') {
    if (str[i] == '\n') {
      PrintError(2, WEBSERV, "Line shoud be end with ';'");
    }
    i++;
    ret++;
  }
  return (ret);
}

void ChangeEvents(std::vector<struct kevent>& change_list, uintptr_t ident,
                  int16_t filter, uint16_t flags, uint16_t fflags,
                  intptr_t data, void* udata) {
  struct kevent temp_event;

  EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
  change_list.push_back(temp_event);
  return;
}

void DeleteUdata(s_base_type* data) {
  switch (data->GetType()) {
    case SERVER: {
      s_server_type* temp = static_cast<s_server_type*>(data);
      delete temp;
      break;
    }
    case CLIENT: {
      s_client_type* temp = static_cast<s_client_type*>(data);
      s_work_type* file = temp->GetChildWork();
      if (file != NULL) {
        delete file;
      }
      delete temp;
      break;
    }
    case WORK: {
      s_work_type* file = static_cast<s_work_type*>(data);
      delete file;
      break;
    }
  }
}

void SetSockoptReuseaddr(int* socket_fd, int socket_length) {
  int bf;
  int ret;

  for (int i = 0; i < socket_length; i++) {
    ret = setsockopt(socket_fd[i], SOL_SOCKET, SO_REUSEADDR, &bf, sizeof(bf));
    if (ret == -1) {
      PrintError(2, WEBSERV, "Server socket option setting is failed");
    }
  }
}

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

std::string HeaderChecker(s_client_type* client, std::string string) {
  t_html responsemsg = client->GetRequest();
  if (responsemsg.header_.find(string) != responsemsg.header_.end()) {
    std::string content_type = responsemsg.header_.find(string)->second;
    return (content_type);
  } else
    return ("");
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

  msg.init_line_.insert({"version", "HTTP/1.1"});
  msg.init_line_.insert({"code", str_code});
  msg.header_.insert({"Date :", date_str});
  msg.header_.insert({"Server :", "webserv/0.1"});
  std::string size = stToString(client->GetResponse().entity_length_);
  // TODO:entity length 필요
  msg.header_.insert({"Content-Length :", size});
  std::string header_str = HeaderChecker(client, "Accept");
  msg.header_.insert({"Content-Type :", header_str});
  msg.header_.insert({"Connection :", "keep-alive"});
  return (msg);
  // TODO: Expire
  // TODO: Cache-Control

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
