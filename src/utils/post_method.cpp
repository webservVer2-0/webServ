#include "../../include/webserv.hpp"

std::string MakeFileName(s_client_type* client) {
  std::string upload_path =
      client->GetConfig().main_config_.find("upload_path")->second;
  std::string ret = "";

  ret.append(upload_path);
  ret.append(client->GetCookieId());
  ret.push_back('_');
  ret.append("userdata.dat");
  return ret;
}

const char* MakeFileDirectory(std::string& uri) {
  const char* dir = uri.c_str();

  while (IsExistFile(dir)) {
    // TODO: file name change
  }
  return dir;
}

void ClientFilePost(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  client->SetErrorCode(NO_ERROR);

  std::string uri = MakeFileName(client);
  // TODO: 먼저 file exist 검사 (반복문 검사);
  const char* dir = MakeFileDirectory(uri);

  int save_file_fd = open(dir, O_RDWR | O_CREAT);
  if (save_file_fd == -1) {
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_FIN);
    return;
  }
  std::vector<struct kevent> tmp;
  s_base_type* new_work = client->CreateWork(&(uri), save_file_fd, file);
  ChangeEvents(tmp, client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0, client);
  ChangeEvents(tmp, save_file_fd, EVFILT_READ, EV_ADD, 0, 0, new_work);
  client->SetErrorCode(OK);
  client->SetStage(POST_START);

  return;
}

void ClientCGIPost(struct kevent* event) {
  (void)event;
  return;
}

void WorkFilePost(struct kevent* event) {
  s_work_type* to_do = static_cast<s_work_type*>(event->udata);
  s_client_type* client = static_cast<s_client_type*>(to_do->GetClientPtr());
  client->SetErrorCode(NO_ERROR);
  size_t write_result = 0;

  write_result = write(to_do->GetFD(), client->GetResponse().entity_,
                       client->GetRequest().entity_length_);
  if (write_result != client->GetRequest().entity_length_) {
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_FIN);
    return;
  }

  std::vector<struct kevent> tmp;
  ChangeEvents(tmp, to_do->GetFD(), EV_DELETE, EV_ADD | EV_EOF, 0, 0, NULL);
  to_do->ChangeClientEvent(EVFILT_WRITE, EV_ENABLE, 0, 0, client);
  close(to_do->GetFD());
  client->SetErrorCode(OK);
  client->SetStage(POST_FIN);
  // TODO: POST 파일 저장 후 검사가 필요한지 확인하기

  return;
}

void WorkCGIPost(struct kevent* event) {
  (void)event;
  return;
}
