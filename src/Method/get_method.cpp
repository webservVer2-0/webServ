#include "../../include/webserv.hpp"

// TODO : seterrorcode, setstage 묶는 함수 / error 처리 과정 묶 함수 만들까 고민

void  MethodGetSetEntity(s_client_type*& client)
{
  client->GetResponse().entity_length_ = GetFileSize(client->GetConvertedURI().c_str());
  try {
    client->GetResponse().entity_ = new char[client->GetResponse().entity_length_ + 1]();
  } catch (const std::exception& e) {
    client->SetErrorString(errno, "get_method.cpp / MethodGetSetEntity안의 new()");
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
    int req_fd = open(uri.c_str(), O_RDONLY | O_NONBLOCK);
    if (req_fd == -1) {
      client->SetErrorString(errno, "get_method.cpp / MethodGetReady()안의 open()");
      client->SetErrorCode(SYS_ERR);
      client->SetStage(ERR_FIN);
      return;
    }
    MethodGetSetEntity(client);
    ServerConfig::ChangeEvents(req_fd, EVFILT_READ, EV_ADD, 0, 0, client);
    ServerConfig::ChangeEvents(req_fd, EVFILT_READ, EV_DISABLE, 0, 0,
                               client);
    ServerConfig::ChangeEvents(req_fd, EVFILT_WRITE, EV_DISABLE, 0, 0,
                               client);
    client->SetErrorCode(OK);
    client->SetStage(GET_START);
  }
  
  return;
}

void ClientGet(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);

  // auto index;
  if (client->GetConfig().index_mode_ != off &&
      client->GetLocationConfig().index_mode_ != off) {
  }
  // TODO : auto index는 나중에. haryu님이 구현하시는 중. delete랑 거의
  // 비슷해서 config_map config = client->GetLocationConfig().main_config_; if
  // (config.find("redirection") != config.end()) {
  //   // redir;
  //   // TODO : redir도 나중에
  // }
  MethodGetReady(client);
}

void WorkGet(struct kevent* event) {
  s_client_type*  client = static_cast<s_client_type*>(event->udata);
  int req_fd = client->GetFD();
  size_t  entity_len = client->GetResponse().entity_length_;
  int idx;
  size_t  read_ret = 0;
  read_ret = read(req_fd, client->GetResponse().entity_, BUF_SIZE);
  if (read_ret == (size_t)-1 || read_ret)
  {
    for (idx = 0; (size_t)idx < read_ret; idx++)
    {
      client->GetVec().push_back(client->GetResponse().entity_[idx]);
    }
    return ;
  }
  if (client->GetVec().size() != entity_len)
  {
    client->SetErrorString(errno, "get_method.cpp / WorkGet()안의 read()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_FIN);

    return ;
  }
  std::vector<char>::iterator it = client->GetVec().begin();
  for (idx = 0; (idx < BUF_SIZE) && (it != client->GetVec().end()); it++)
  {
    client->GetResponse().entity_[idx] = *(it++);
  }
  client->GetResponse().entity_[idx] = '\0';

  client->SetErrorCode(OK);
  client->SetMimeType(client->GetConvertedURI());
  size_t chunk_size =
      atoi(client->GetConfig().main_config_.find(BODY)->second.c_str());
  if (entity_len > chunk_size) {
    client->SetStage(GET_CHUNK);
  } else {
    ServerConfig::ChangeEvents(req_fd, EVFILT_READ, EV_DISABLE, 0, 0, client);
    ServerConfig::ChangeEvents(req_fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    ServerConfig::ChangeEvents(req_fd, EVFILT_WRITE, EV_ENABLE, 0, 0, client);//TODO : EV_ENABLE만 해도되지않나?
    client->SetStage(GET_FIN);
  }
  if (close(req_fd) == -1) {
    client->SetErrorString(errno, "get_method.cpp / WorkGet()안의 close()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(ERR_FIN);
  }

  return;
}
