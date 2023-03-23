#include "../../include/webserv.hpp"

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
    client->SetStage(ERR_FIN);
  }
}

/**
 * @brief GET_READY일때 캐시파일인지 여부 확인, 일반 파일인 경우 open()하고,
 * read event 등록.
 *
 * @param client : udata를 s_client_type*&로 형변환한것
 */
void MethodGetReady(s_client_type*& client) {
  std::string uri = client->GetConvertedURI();
  t_http& response = client->GetResponse();

  if (client->GetCachePage(uri, response))  // 캐시파일인경우
  {
    client->SetMimeType(uri);
    client->SetErrorCode(OK);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                               client);
    client->SetStage(GET_FIN);

    return;
  } else  // 일반파일인경우
  {
    int file_fd = open(uri.c_str(), O_RDONLY | O_NONBLOCK);
    if (file_fd == -1) {
      client->SetErrorString(errno,
                             "get_method.cpp / MethodGetReady()안의 open()");
      client->SetErrorCode(SYS_ERR);
      client->SetStage(ERR_FIN);

      return;
    }
    // kick
    MethodGetSetEntity(client);
    client->SetErrorCode(OK);
    client->SetStage(GET_START);
    s_work_type* work =
        static_cast<s_work_type*>(client->CreateWork(&uri, file_fd, file));
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(file_fd, EVFILT_READ, EV_ADD, 0, 0, work);
  }

  return;
}

void ClientGet(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  std::string uri(client->GetConvertedURI());
  std::string dir(uri.substr(0, uri.rfind('/')));

  if ((client->GetConfig().index_mode_ != off) ||
      (client->GetLocationConfig().index_mode_ != off)) {
    if (uri.find(".html") != std::string::npos)  // TODO : 디렉 구조일땐?
    {
      MakeAutoindexPage(client->GetResponse(), dir);

      client->SetMimeType(uri);
      client->SetErrorCode(OK);
      client->SetStage(GET_FIN);
      ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                                 client);
      ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                                 client);

      return;
    }
  }

  std::cout << "uri : " << dir << std::endl;
  if (uri.find("/delete") != std::string::npos) {
    // config delete.html
    printf("here\n");
    // MakeDeletePage(client, client->GetResponse(), uri);
    std::cout << dir << std::endl;
    MakeDeletePage(client, client->GetResponse(), dir);
    printf("here2\n");

    client->SetMimeType(uri);
    client->SetErrorCode(OK);
    client->SetStage(GET_FIN);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                               client);
    return;
  }
  // auto index랑 비슷햐

  config_map config = client->GetLocationConfig().main_config_;
  if (config.find("redirection") != config.end()) {
    // redir;
    // TODO : redir
  }
  MethodGetReady(client);
}

void WorkGet(struct kevent* event) {
  s_work_type* work = static_cast<s_work_type*>(event->udata);
  s_client_type* client =
      static_cast<s_client_type*>(work->GetClientPtr());  // client->base

  int file_fd = work->GetFD();
  t_http* response = &(work->GetResponseMsg());
  size_t idx;
  size_t read_ret = 0;
  read_ret = read(file_fd, response->entity_, response->entity_length_);  //
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
    client->SetStage(ERR_FIN);

    return;
  }
  std::vector<char>::iterator it = work->GetVec().begin();
  for (idx = 0; idx < response->entity_length_; idx += 2) {
    response->entity_[idx] = *(it++);
    if (idx + 1 < response->entity_length_) {
      response->entity_[idx + 1] = *(it++);
    }
  }

  client->SetErrorCode(OK);
  client->SetMimeType(work->GetUri());

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
