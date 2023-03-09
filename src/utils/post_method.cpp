#include "../../include/webserv.hpp"

t_error ClientPost(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  t_error error_code = NO_ERROR;

  // TODO: 저장할 파일 경로가 request_msg 의 어느 부분에 있는지 확인할것.
  // 현재는 init_line애 있는지 header에 있는지 알수 없음.
  // 일단 작성은 init_line으로 하고 있다고 간주함.
  //   const char* dir =
  //   client->GetRequest().init_line_.find("uri")->first.c_str();
  char* dir = "./index.html";

  int save_file_fd = open(dir, O_RDWR | O_CREAT);
  if (save_file_fd == -1) {
    // error;
    // error_code = SYS_ERR;
    // return error_code;
  }
  std::vector<struct kevent> tmp;
  client->SetStage(
      POST_READY);  // TODO: new_work의 stage 변경을 어떻게 해줄지 결정하기
  s_base_type* new_work = client->CreateWork(
      reinterpret_cast<std::string*>(dir), save_file_fd, file);
  ChangeEvents(tmp, client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0, client);
  ChangeEvents(tmp, save_file_fd, EVFILT_READ, EV_ADD, 0, 0, new_work);

  return error_code;
}

t_error WorkPost(struct kevent* event) {
  s_work_type* to_do = static_cast<s_work_type*>(event->udata);
  s_client_type* client = static_cast<s_client_type*>(to_do->GetClientPtr());
  t_error error_code = NO_ERROR;
  size_t write_result = 0;

  write_result = write(to_do->GetFD(), client->GetResponse().entity_,
                       client->GetRequest().entity_length_);
  if (write_result != client->GetRequest().entity_length_) {
    // error;
    // error_code = SYS_ERR;
    // return error_code;
  }
  std::vector<struct kevent> tmp;

  close(to_do->GetFD());
  ChangeEvents(tmp, to_do->GetFD(), EV_DELETE, EV_ADD | EV_EOF, 0, 0, NULL);
  client->SetStage(
      POST_FIN);  // TODO: POST 파일 저장 후 검사가 필요한지 확인하기
  to_do->ChangeClientEvent(EVFILT_WRITE, EV_ENABLE, 0, 0,
                           client);  // TODO: 이 syntax 맞는지 확인하기;

  return error_code;
}
