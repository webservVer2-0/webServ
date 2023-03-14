#include "../../include/webserv.hpp"

void ClientGet(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  client->SetErrorCode(NO_ERROR);
  const char* dir;
  std::string uri = client->GetLocationConfig().location_;

  int req_fd = open(dir, O_RDONLY);
  if (req_fd == -1) {
    client->SetErrorCode(SYS_ERR);
    return;
  }
  fcntl(req_fd, F_SETFL, O_NONBLOCK);

  s_base_type* work = client->CreateWork(&uri, req_fd, file);
  std::vector<struct kevent> tmp;
  ChangeEvents(tmp, req_fd, EVFILT_READ, EV_ADD, 0, 0, work);
  client->SetStage(GET_START);

  return;
}

void WorkGet(struct kevent* event) {
  s_work_type* work = static_cast<s_work_type*>(event->udata);
  s_client_type* client = static_cast<s_client_type*>(work->GetClientPtr());
  size_t chunk_size = static_cast<size_t>(
      client->GetConfig().main_config_.find(BODY)->second.size());
  client->SetErrorCode(NO_ERROR);

  work->GetResponseMsg().entity_length_ = event->data;
  size_t tmp_entity_len = work->GetResponseMsg().entity_length_;
  work->GetResponseMsg().entity_ = new char[tmp_entity_len];
  size_t read_ret = 0;
  int req_fd = work->GetFD();
  read_ret = read(req_fd, work->GetResponseMsg().entity_, tmp_entity_len);
  if ((read_ret != tmp_entity_len) || read_ret == -1) {
    client->SetErrorCode(SYS_ERR);
    return;
  }
  work->ChangeClientEvent(EVFILT_WRITE, EV_ADD, 0, 0, client);
  work->ChangeClientEvent(EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, work);
  if (close(req_fd) == -1) {
    client->SetErrorCode(SYS_ERR);
    return;
  }
  client->SetErrorCode(OK);
  if (tmp_entity_len > chunk_size)
    work->SetClientStage(GET_CHUNK);
  else
    work->SetClientStage(GET_FIN);

  return;
