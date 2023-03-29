#include "../../include/webserv.hpp"
#include "dirent.h"

// TODO : seterrorcode, setstage 묶는 함수 / error 처리 과정 묶 함수 만들까 고민

void MethodGetSetEntity(s_client_type*& client) {
  client->GetResponse().entity_length_ =
      GetFileSize(client->GetConvertedURI().c_str());
  try {
    client->GetResponse().entity_ =
        new char[client->GetResponse().entity_length_]();
  } catch (const std::exception& e) {
    client->SetErrorString(errno,
                           "get_method.cpp / MethodGetSetEntity안의 new()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_READY);
  }
}

/**
 * @brief GET_READY일때 캐시파일인지 여부 확인, 일반 파일인 경우 open()하고,
 * read event 등록.
 *
 * @param client : udata를 s_client_type*&로 형변환한것
 */
void MethodGetReady(s_client_type*& client) {
  std::string converted_uri = client->GetConvertedURI();
  t_http& response = client->GetResponse();

  if (client->GetCachePage(converted_uri, response))  // 캐시파일인경우
  {
    client->SetMimeType(converted_uri);
    client->SetErrorCode(OK);
    client->SetStage(GET_FIN);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                               client);
    return;
  } else  // 일반파일인경우
  {
    if (opendir(converted_uri.c_str()) != NULL) { //auto index 꺼져있고 default file 없을때
      client->SetErrorCode(FORBID);
      client->SetStage(GET_FIN);

      return;
    }
    int file_fd = open(converted_uri.c_str(), O_RDONLY | O_NONBLOCK);
    if (file_fd == -1) {
      client->SetErrorString(errno,
                             "get_method.cpp / MethodGetReady()안의 open()");
      client->SetErrorCode(SYS_ERR);
      client->SetStage(ERR_READY);

      return;
    }
    MethodGetSetEntity(client);
    client->SetErrorCode(OK);
    client->SetStage(GET_START);
    s_work_type* work =
        static_cast<s_work_type*>(client->CreateWork(&converted_uri, file_fd, file));
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(file_fd, EVFILT_READ, EV_ADD, 0, 0, work);
  }

  return;
}

void ClientGet(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  std::string converted_uri(client->GetConvertedURI());
  t_server server_config = client->GetConfig();
  t_loc loc_config = client->GetLocationConfig();
  if (loc_config.index_mode_ == on) {
    if (opendir(converted_uri.c_str()) != NULL)
    {
      MakeAutoindexPage(client, client->GetResponse(), converted_uri);

      client->SetMimeType(converted_uri);
      client->SetErrorCode(OK);
      client->SetStage(GET_FIN);
      ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                                 client);
      ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                                 client);

      return;
    }
  }
  if (converted_uri.find("/delete") != std::string::npos) {
    MakeDeletePage(client, client->GetResponse(),
                   server_config.main_config_.find(UPLOAD)->second);
    client->SetMimeType(converted_uri);
    client->SetErrorCode(OK);
    client->SetStage(GET_FIN);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                               client);

    return;
  }

  if (converted_uri.find("redirection") != std::string::npos) {
    client->SetErrorCode(MOV_PERMAN);
    client->SetStage(GET_FIN);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                               client);

    return;
  }

  MethodGetReady(client);
}

void WorkGet(struct kevent* event) {
  s_work_type* work = static_cast<s_work_type*>(event->udata);
  s_client_type* client = static_cast<s_client_type*>(work->GetClientPtr());

  int file_fd = work->GetFD();
  t_http* response = &(work->GetResponseMsg());
  size_t idx;
  size_t read_ret = read(file_fd, response->entity_, response->entity_length_);
  if (read_ret == (size_t)-1 || read_ret) {
    for (idx = 0; (idx < read_ret) && (idx < response->entity_length_); idx++) {
      work->GetVec().push_back(response->entity_[idx]);
    }
    if (work->GetVec().size() != response->entity_length_) {
      return;
    }
  }
  if (work->GetVec().size() != response->entity_length_) {
    client->SetErrorString(errno, "get_method.cpp / WorkGet()안의 read()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_READY);

    return;
  }
  client->SetErrorCode(OK);
  client->SetMimeType(work->GetUri());

  std::vector<char>::iterator it = work->GetVec().begin();
  for (idx = 0; idx < response->entity_length_; idx += 2) {
    response->entity_[idx] = *(it++);
    if (idx + 1 < response->entity_length_) {
      response->entity_[idx + 1] = *(it++);
    }
  }

  size_t chunk_size =
      atoi(client->GetConfig().main_config_.find(BODY)->second.c_str());
  if (response->entity_length_ > chunk_size) {
    client->SetStage(GET_CHUNK);
  } else {
    ServerConfig::ChangeEvents(file_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    client->SetStage(GET_FIN);
    work->ChangeClientEvent(EVFILT_WRITE, EV_ENABLE, 0, 0, client);
  }
  close(file_fd);

  return;
}
