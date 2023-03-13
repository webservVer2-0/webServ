#include "../../include/webserv.hpp"

// TODO : err_custom_ getter, setter 만들까?
// TODO : error 발생시 err_custom_ 지정해줘야함
// CreateWork()시 첫번째 인수 왜 포인터? > reference로 할까 했는데 const 때문에 복잡해서 std::string*로 설정하심
// s_work_type에서 SetError 필요하지 않을까? > 괜춘. client에 해주기때문에
// s_work_type client_ptr_ 언제 쓰지? > 일단 에러 지정하고, kevent 등록할때

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
    // entity_ 할당해제 여기서 X
    return ;
  }
  // TODO : read 성공시 200으로 바꿔줘야함

  work->ChangeClientEvent(EVFILT_READ, EV_DISABLE, 0, 0, client);
  // work->ChangeClientEvent(EVFILT_WRITE, EV_ADD, 0, 0, work);
  // TODO : write enable 해줘야함
  work->SetClientStage(GET_FIN);
  // TODO : delete도 해줘야함
  close(req_fd);

  return ;
}  // TODO : t_error 반환형말고 void로 해도 됨. 어차피 client에 들어있으니까
