#include "../../include/utils.hpp"

#include "../../include/config.hpp"

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
    if (isalnum(target[pos]) || (target[pos] == '_') || target[pos] == '/' ||
        target[pos] == '.') {
      pos++;
      ret++;
    } else {
      PrintLine(str, pos);
      PrintError(4, WEBSERV, CRITICAL, "Code Error \"utils.cpp\" - 86 line",
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

void DeleteUdata(s_base_type* data) {
  if (data->GetType() != CLIENT) return;
  s_client_type* temp = static_cast<s_client_type*>(data);
  if (temp->GetChildWork() != NULL) {
    s_work_type* file = temp->GetChildWork();
    if (file != NULL) {
      delete file;
    }
  }

  if (temp->GetRequest().entity_length_ != 0)
    delete[] temp->GetRequest().entity_;
  temp->GetRequest().entity_length_ = 0;
  temp->GetRequest().header_.clear();
  temp->GetRequest().init_line_.clear();
  if (temp->GetResponse().entity_length_ != 0)
    delete[] temp->GetResponse().entity_;
  temp->GetResponse().entity_length_ = 0;
  temp->GetResponse().header_.clear();
  temp->GetResponse().init_line_.clear();
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

void ResetConnection(s_client_type* udata) {
  // s_client_type data clear
  udata->GetTimeData()[0] = 0;
  udata->GetTimeData()[1] = 0;
  udata->SetConfigPtr(NULL);

  const_cast<std::string&>(udata->GetOriginURI()).clear();

  t_http* temp_http = &(udata->GetRequest());
  if (temp_http->entity_ != NULL) {
    delete[] temp_http->entity_;
  }
  temp_http->entity_length_ = 0;
  temp_http->header_.clear();
  temp_http->init_line_.clear();

  temp_http = &((udata)->GetResponse());
  if (temp_http->entity_ != NULL) {
    delete[] temp_http->entity_;
  }
  temp_http->entity_length_ = 0;
  temp_http->header_.clear();
  temp_http->init_line_.clear();

  udata->GetMimeType().clear();
  udata->SetStage(DEF);
  udata->SetErrorCode(NO_ERROR);
  udata->SetErrorString(0, std::string());
  //   if (udata->GetChildWork() != NULL) {
  //     delete udata->GetChildWork();
  //   }
}
