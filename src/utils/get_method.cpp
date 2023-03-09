#include "../../include/webserv.hpp"

// TODO : err_custom_ getter, setter 만들까?
// TODO : error 발생시 err_custom_ 지정해줘야함
// TODO : CreateWork()시 첫번째 인수 왜 포인터?
// TODO : s_work_type에서 SetError 필요하지 않을까?
// TODO : s_work_type client_ptr_ 언제 쓰지? 일단 에러 지정할때만 씀

/**
 * @brief
 *
 * @param event
 * @return t_error
 *
 */

t_error ClientGet(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  client->SetErrorCode(NO_ERROR);
  const char* dir;  // TODO : (request uri).c_str(const char*)

  int req_fd = open(dir, O_RDONLY);
  if (req_fd == -1) {
    client->SetErrorCode(SYS_ERR);
    // return client->GetErrorCode(); TODO : return?
  }
  fcntl(req_fd, F_SETFL, O_NONBLOCK);

  s_base_type* work = client->CreateWork(request uri, req_fd, file);
  std::vector<struct kevent> tmp;
  ChangeEvents(tmp, req_fd, EVFILT_READ, EV_ADD, 0, 0, work);
  client->SetStage(GET_READY);

  return client->GetErrorCode();
}

t_error WorkGet(struct kevent* event) {
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
    // TODO : entity_ 할당해제 여기서?
    // TODO : return client->GetErrorCode(); ?
  }
  close(req_fd);

  work->ChangeClientEvent(
      EVFILT_READ, EV_ADD, 0, 0,
      work);  // TODO : work? < error_code_ 설정안돼있는채로 등록됨
  work->SetClientStage(GET_FIN);

  return client->GetErrorCode();
}
