#include "../../include/datas.hpp"

/****************** Base Type ********************/

s_base_type::s_base_type(int fd) : type_(SERVER), fd_(fd) {}

void s_base_type::SetType(t_event val) { type_ = val; }

t_event s_base_type::GetType() const { return this->type_; }

int s_base_type::GetFD() { return this->fd_; }

/****************** Server Type ********************/

s_server_type::s_server_type(ServerConfig& config_list, int server_number,
                             int server_fd)
    : s_base_type(server_fd) {
  this->self_config_ =
      const_cast<t_server*>(&config_list.GetServerList(server_number));
}

s_server_type::~s_server_type() {}

s_base_type* s_server_type::CreateClient(int client_fd) {
  s_client_type* child;
  child = new s_client_type(this->self_config_, client_fd, this);
  return child;
}

/****************** Client Type ********************/

s_client_type::s_client_type(t_server* config, int client_fd,
                             s_server_type* mother)
    : s_base_type(client_fd) {
  cookie_id_ = rand();
  this->SetType(CLIENT);
  this->config_ptr_ = config;
  parent_ptr_ = mother;
  data_ptr_ = NULL;
  stage_ = DEF;
  status_code_ = NO_ERROR;
}

s_client_type::~s_client_type() {}

s_base_type* s_client_type::CreateWork(std::string* path, int file_fd,
                                       s_chore work_type) {
  s_work_type* work;

  work = new s_work_type(*path, file_fd, work_type);
  work->SetType(WORK);
  work->SetClientPtr(this);
  this->data_ptr_ = work;
  return (work);
}

int s_client_type::GetCookieId(void) { return this->cookie_id_; }

t_html& s_client_type::GetRequest(void) { return this->request_msg_; }
t_html& s_client_type::GetResponse(void) { return this->response_msg_; }

const t_stage& s_client_type::GetStage(void) { return this->stage_; }
void s_client_type::SetStage(t_stage val) { this->stage_ = val; }

const t_error& s_client_type::GetErrorCode(void) { return this->status_code_; }
void s_client_type::SetErrorCode(t_error val) { this->status_code_ = val; }

const t_server& s_client_type::GetConfig(void) { return *this->config_ptr_; }
const s_server_type& s_client_type::GetParentServer(void) {
  return *(dynamic_cast<s_server_type*>(parent_ptr_));
}
const s_work_type& s_client_type::GetChildWork(void) {
  return *(dynamic_cast<s_work_type*>(data_ptr_));
}

/****************** Work Type ********************/

s_work_type::s_work_type(std::string& path, int fd, s_chore work_type)
    : s_base_type(fd), uri_(path), work_type_(work_type) {}

s_work_type::~s_work_type() {}

void s_work_type::SetClientPtr(s_base_type* ptr) { this->client_ptr_ = ptr; }

s_base_type* s_work_type::GetClientPtr(void) { return this->client_ptr_; }

const std::string& s_work_type::GetUri(void) { return this->uri_; }

s_chore s_work_type::GetWorkType(void) { return this->work_type_; }
