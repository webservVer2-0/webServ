#include "../../include/webserv.hpp"

// TODO : err_custom_ getter, setter 만들까?
// TODO : error 발생시 err_custom_ 지정해줘야함
// TODO : 예외처리 더 ?

/**
 * @brief
 *
 * @param event
 * @return t_error
 *
 */

void ClientGet(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  client->SetErrorCode(NO_ERROR);
  const char* dir;  // TODO : (request uri).c_str(const char*)

  int req_fd = open(dir, O_RDONLY);
  if (req_fd == -1) {
    client->SetErrorCode(SYS_ERR);
    return ;
  }
  fcntl(req_fd, F_SETFL, O_NONBLOCK);

  s_base_type* work = client->CreateWork(request uri, req_fd, file);
  std::vector<struct kevent> tmp;
  ChangeEvents(tmp, req_fd, EVFILT_READ, EV_ADD, 0, 0, work);
  client->SetStage(GET_READY);

  return ;
}

void WorkGet(struct kevent* event) {
  s_work_type* work = static_cast<s_work_type*>(event->udata);
  s_client_type* client = static_cast<s_client_type*>(work->GetClientPtr());
  client->SetErrorCode(NO_ERROR);

  work->GetResponseMsg().entity_length_ = event->data;
  size_t tmp_entity_len = work->GetResponseMsg().entity_length_;
  work->GetResponseMsg().entity_ =
      new char[tmp_entity_len];  // TODO : new error시
  size_t read_ret = 0;
  int req_fd = work->GetFD();
  read_ret = read(req_fd, work->GetResponseMsg().entity_, tmp_entity_len);
  if ((read_ret != tmp_entity_len) || read_ret == -1) {
    client->SetErrorCode(SYS_ERR);
    // 파일 그냥 보내주면 돼서 할당안해도 되지 않나싶은데 물어보기 > get말고 post에 해당
    return ;
  }

  work->ChangeClientEvent(EVFILT_WRITE, EV_ADD, 0, 0, work);
  work->ChangeClientEvent(EVFILT_READ, EV_DISABLE | EV_DELETE, 0, 0, client);
  // TODO : delete close 관계 더 찾아보기
  if (close(req_fd) == -1);
  {
    client->SetErrorCode(SYS_ERR);
    return ;
  }
  client->SetErrorCode(OK);
  // read 성공시 200으로 바꿔줘야함 < 이거 좀 더 물어봐야 함 >> 안물어봐도됨 200으로 바꾸고 끝임
  work->SetClientStage(GET_FIN);

  return ;
}