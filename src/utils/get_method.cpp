#include "../../include/webserv.hpp"

// TODO : err_custom_ getter, setter 만들까?
// TODO : error 발생시 err_custom_ 지정해줘야함
// TODO : CreateWork()시 첫번째 인수 왜 포인터?
// TODO : s_work_type에서 SetError 필요하지 않을까?

/**
 * @brief 
 * 
 * @param event 
 * @return t_error 
 * 
 */

t_error ClientGet(struct kevent* event)
{
	s_client_type* client = static_cast<s_client_type*>(event->udata);
	client->SetErrorCode(NO_ERROR);
	std::vector<struct kevent> tmp;
	
	const char* dir; //(request_msg_ 안의 uri).c_str(const char*)

	int	req_fd = open(dir, O_RDONLY);

	if (req_fd == -1)
	{
		client->SetErrorCode(SYS_ERR);
		return client->GetErrorCode();
	}
	fcntl(req_fd, F_SETFL, O_NONBLOCK);

	s_base_type* work = client->CreateWork(request uri, req_fd, file);

	ChangeEvents(tmp, req_fd, EVFILT_READ, EV_ADD, 0, 0, work); // ev_disable? ev_add?
	client->SetStage(GET_READY);

	return client->GetErrorCode();
}

t_error WorkGet(struct kevent* event)
{
	s_work_type* work = static_cast<s_work_type*>(event->udata);
	// TODO : NO_ERROR로 error setting 필요
	int	req_fd = work->GetFD();
	size_t file_len = event->data;
  size_t read_ret = 0;
	std::vector<struct kevent> tmp;

  work->GetResponseMsg().entity_ = new char[file_len]; // TODO : new error시
  read_ret = read(req_fd, work->GetResponseMsg().entity_, file_len);
  if ((read_ret != file_len) || read_ret == -1)
  {
    // TODO : work->SetErrorCode(SYS_ERR);
    // TODO : entity_ 할당해제 여기서?
    // TODO : return work->GetErrorCode();
  }
  close(req_fd);

  ChangeEvents(tmp, work->GetClientPtr()->GetFD(), EVFILT_READ, EV_ADD, 0, 0, work); // work?
  // TODO : return work->GetErrorCode();
}
