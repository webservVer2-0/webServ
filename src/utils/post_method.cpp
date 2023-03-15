// #include "../../include/webserv.hpp"

// t_error ClientPost(struct kevent* event) {
//   s_client_type* client = static_cast<s_client_type*>(event->udata);
//   t_error error_code = NO_ERROR;

//   // TODO: 저장할 파일 경로가 request_msg 의 어느 부분에 있는지 확인할것.
//   // 현재는 init_line애 있는지 header에 있는지 알수 없음.
//   // 일단 작성은 init_line으로 하고 있다고 간주함.
//   //   const char* dir =
//   // client->GetRequest().init_line_.find("uri")->second.c_str();
//   char* dir = "./index.html";

//   int save_file_fd = open(dir, O_RDWR | O_CREAT);
//   if (save_file_fd == -1) {
//     // error;
//     // error_code = SYS_ERR;
//     // return error_code;
//   }
//   std::vector<struct kevent> tmp;
//   s_base_type* new_work =
//       client->CreateWork(&(std::string(dir)), save_file_fd, file);
//   ChangeEvents(tmp, client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0, client);
//   ChangeEvents(tmp, save_file_fd, EVFILT_READ, EV_ADD, 0, 0, new_work);
//   client->SetStage(POST_READY);

//   // TODO: return 값을 void로 수정하고, client->GetErrorCode()만 바꿔주기;
//   return error_code;  // return client->GetErrorCode();
// }

// t_error WorkPost(struct kevent* event) {
//   s_work_type* to_do = static_cast<s_work_type*>(event->udata);
//   s_client_type* client = static_cast<s_client_type*>(to_do->GetClientPtr());
//   t_error error_code = NO_ERROR;
//   size_t write_result = 0;

//   write_result = write(to_do->GetFD(), client->GetResponse().entity_,
//                        client->GetRequest().entity_length_);
//   if (write_result != client->GetRequest().entity_length_) {
//     // error;
//     // error_code = SYS_ERR;
//     // return error_code;
//   }
//   std::vector<struct kevent> tmp;

//   ChangeEvents(tmp, to_do->GetFD(), EV_DELETE, EV_ADD | EV_EOF, 0, 0, NULL);
//   client->SetStage(
//       POST_FIN);  // TODO: POST 파일 저장 후 검사가 필요한지 확인하기
//   to_do->ChangeClientEvent(EVFILT_WRITE, EV_ENABLE, 0, 0, client);
//   close(to_do->GetFD());

//   // TODO: 캐시페이지 추가?

//   return error_code;
// }
