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


/*
t_error Open_File(t_error error_code)
{
  FILE* in_file = NULL;

  in_file = fopen("text file", "r");

  if (in_file == NULL) {
	std::cout << "Error: 읽기 실패" << std::endl;
	fclose(in_file);
	error_code = SYS_ERR; //sys_err?
  }
  return error_code;
}

t_error Get_File_Size(t_error error_code)
{
  if (fseek(in_file, 0, SEEK_END))
  {
	error_code = SYS_ERR;
	//바로 return ?
  }
  if ((buf_size = static_cast<size_t>(ftell(in_file))) == -1L)
  {
	error_code = SYS_ERR;
	//바로 return ?
  }

  rewind(in_file); // error 처리 해야함

  return error_code;
}

t_error Read_File(t_error error_code)
{
  char* buf = new char[buf_size];
  if (buf == NULL) {
	error_code = SYS_ERR;
  }

  result = fread(buf, sizeof(char), buf_size, in_file);
  if (result != buf_size) {
	std::cout << "Error: 읽기 " << std::endl;
	error_code = SYS_ERR;
  }

  client_type->response_msg_.entity_length_ = ;
  return error_code;
}

Get_Method(struct kevent *)로 수정
t_error Get_Method(void* udata) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);//base type으로 형변환하고 client 인지 file인지 확인
  t_error error_code = NO_ERROR;

  //조건문 쓰는게 더 좋을것같음 switch case 말고
  switch (client_type->GetStage()) {
	case REQ_FIN:
	  // open();
	  if (error_code = Open_File())
	  {
		//에러처리 어떻게?
		//response에 보내야할것같은데 뭘 보내지?
	  }
	  else
	  {
		client_type->SetStage(GET_READY);
		// kevent 등록 
	  }
	  break;
	case GET_READY:
	  if (error_code = Read_File())
	  {

	  }
	  //file close
	  //delete[] entity;
	  break;
	default: //없어도 될듯..?
	  break;
  }


  return error_code;
}

#include <cstdio>
#include <iostream>

void binary_test() {
  FILE* in_file = NULL;
  size_t buf_size;
  size_t result;

  in_file = fopen("a.out", "rb");

  if (in_file == NULL) {
	std::cout << "Error: 읽기 실패" << std::endl;
	fclose(in_file);
	exit(1);
  }

  fseek(in_file, 0, SEEK_END);
  buf_size = static_cast<size_t>(ftell(in_file));
  rewind(in_file);

  char* buf = new char[buf_size];
  if (buf == NULL) {
	exit(1);
  }

  result = fread(buf, 1, buf_size, in_file);
  if (result != buf_size) {
	std::cout << "Error: 읽기 " << std::endl;
  }
  std::cout << "---bin---" << std::endl;
  std::cout << "bin size(): " << buf_size << std::endl;
  std::cout << "result: " << result << std::endl;

  std::cout << "---" << std::endl;

  FILE* out_file = fopen("ret_bin", "w");

  if (out_file == NULL) {
	std::cout << "Error: 파일 생성 실패" << std::endl;
  }
  size_t write_ret;

  write_ret = fwrite(buf, 1, buf_size, out_file);

  std::cout << "write_ret: " << write_ret << std::endl;

  fclose(out_file);
  fclose(in_file);
  delete[] buf;
}

void text_test() {
  FILE* in_file = NULL;
  size_t buf_size;
  size_t result;

  in_file = fopen("index.html", "r");

  if (in_file == NULL) {
	std::cout << "Error: 읽기 실패" << std::endl;
	fclose(in_file);
	exit(1);
  }

  fseek(in_file, 0, SEEK_END);
  buf_size = static_cast<size_t>(ftell(in_file));
  rewind(in_file);

  char* buf = new char[buf_size];
  if (buf == NULL) {
	exit(1);
  }

  result = fread(buf, sizeof(char), buf_size, in_file);
  if (result != buf_size) {
	std::cout << "Error: 읽기 " << std::endl;
  }
  std::cout << "---text---" << std::endl;
  std::cout << "bin size(): " << buf_size << std::endl;
  std::cout << "result: " << result << std::endl;

  std::cout << "---" << std::endl;

  FILE* out_file = fopen("ret_index", "w");

  if (out_file == NULL) {
	std::cout << "Error: 파일 생성 실패" << std::endl;
  }
  size_t write_ret;

  write_ret = fwrite(buf, sizeof(char), buf_size, out_file);

  std::cout << "write_ret: " << write_ret << std::endl;

  fclose(out_file);
  fclose(in_file);
  delete[] buf;
}

int main(void) {
  binary_test();
  text_test();
  return (0);
}
*/