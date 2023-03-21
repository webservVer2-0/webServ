#include "../../include/webserv.hpp"

// TODO : seterrorcode, setstage 묶는 함수 / error 처리 과정 묶 함수 만들까 고민

/**
 * @brief GET_READY일때 캐시파일인지 여부 확인, 일반 파일인 경우 open()하고,
 * read event 등록.
 *
 * @param client : udata를 s_client_type*&로 형변환한것
 */
void MethodGetReady(s_client_type*& client) {
  const char* dir = client->GetRequest().init_line_.find("URI")->second.c_str();
  t_http& response = client->GetResponse();
  if (client->GetCachePage(std::string(dir), response))  // 캐시파일인경우
  {
    client->SetMimeType(std::string(dir));
    client->SetErrorCode(OK);
    client->SetStage(GET_FIN);

    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                               client);
    return;
  } else  // 일반파일인경우
  {
    int req_fd = open(dir, O_RDONLY | O_NONBLOCK);
    if (req_fd == -1) {
      client->SetErrorString(errno, "GET method open()");
      client->SetErrorCode(SYS_ERR);
      client->SetStage(ERR_FIN);
      return;
    }

    s_base_type* work = client->CreateWork(
        &client->GetRequest().init_line_.find("URI")->second, req_fd, file);

    ServerConfig::ChangeEvents(req_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
                               work);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_DISABLE, 0, 0,
                               client);
    client->SetErrorCode(OK);
    client->SetStage(GET_START);
  }
  return;
}

void ClientGet(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);

  // if (client->GetConfig().index_mode_ != off &&
  //     client->GetLocationConfig().index_mode_ != off) {
  //   // auto index;
  // }
  // // TODO : auto index는 나중에. haryu님이 구현하시는 중. delete랑 거의
  // 비슷해서 config_map config = client->GetLocationConfig().main_config_; if
  // (config.find("redirection") != config.end()) {
  //   // redir;
  //   // TODO : redir도 나중에
  // }
  MethodGetReady(client);
}

void WorkGet(struct kevent* event) {
  s_work_type* work = static_cast<s_work_type*>(event->udata);
  s_client_type* client = static_cast<s_client_type*>(work->GetClientPtr());
  size_t chunk_size =
      atoi(client->GetConfig().main_config_.find(BODY)->second.c_str());

  work->GetResponseMsg().entity_length_ = event->data;
  size_t tmp_entity_len = work->GetResponseMsg().entity_length_;

  try {
    work->GetResponseMsg().entity_ = new char[tmp_entity_len];
  } catch (const std::exception& e) {
    client->SetErrorString(errno, "GET method new()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_FIN);
  }

  size_t read_ret = 0;
  int req_fd = work->GetFD();
  read_ret = read(req_fd, work->GetResponseMsg().entity_, tmp_entity_len);
  if ((read_ret != tmp_entity_len) || read_ret == size_t(-1)) {
    client->SetErrorString(errno, "GET method read()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_FIN);
    return;
  }
  client->SetErrorCode(OK);
  client->SetMimeType(work->GetUri());
  if (tmp_entity_len > chunk_size) {
    work->SetClientStage(GET_CHUNK);
  } else {
    work->SetClientStage(GET_FIN);
    work->ChangeClientEvent(EVFILT_READ, EV_DISABLE, 0, 0, client);
    work->ChangeClientEvent(EVFILT_WRITE, EV_ENABLE, 0, 0, client);
    ServerConfig::ChangeEvents(work->GetFD(), EVFILT_READ, EV_DELETE, 0, 0,
                               work);
  }
  if (close(req_fd) == -1) {
    client->SetErrorString(errno, "GET method close()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_FIN);
  }

  return;
}
