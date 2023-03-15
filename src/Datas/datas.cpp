#include "../../include/datas.hpp"

#include <sstream>

typedef std::map<std::string, std::string> config_map;

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
  int l_fd;
  int e_fd;

  l_fd = open(self_config_->main_config_.at("access_log").c_str(),
              O_WRONLY | O_APPEND | O_NONBLOCK, 0644);
  e_fd = open(self_config_->main_config_.at("error_log").c_str(),
              O_WRONLY | O_APPEND | O_NONBLOCK, 0644);

  this->logger_.SetFDs(l_fd, e_fd);
  ChangeEvents(config_list.change_list_, l_fd, EVFILT_WRITE,
               EV_ADD | EV_DISABLE, 0, NULL, &(this->logger_));
  ChangeEvents(config_list.change_list_, e_fd, EVFILT_WRITE,
               EV_ADD | EV_DISABLE, 0, NULL, &(this->logger_));
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
  std::ostringstream temp;
  temp << rand();
  cookie_id_ = temp.str();
  temp.clear();
  this->time_data_[0] = std::time(NULL);
  this->SetType(CLIENT);
  this->config_ptr_ = config;
  this->loc_config_ptr_ = NULL;
  parent_ptr_ = static_cast<s_base_type*>(mother);
  data_ptr_ = NULL;
  sent_size_ = 0;
  stage_ = DEF;
  status_code_ = NO_ERROR;
}

s_client_type::~s_client_type() {}

s_base_type* s_client_type::CreateWork(std::string* path, int file_fd,
                                       s_chore work_type) {
  s_work_type* work;

  work = new s_work_type(*path, file_fd, work_type, response_msg_);
  work->SetType(WORK);
  work->SetClientPtr(this);
  this->data_ptr_ = work;
  return (work);
}

std::string s_client_type::GetCookieId(void) { return this->cookie_id_; }
void s_client_type::SetCookieId(std::string prev_id) {
  this->cookie_id_ = prev_id;
}

t_http& s_client_type::GetRequest(void) { return this->request_msg_; }
t_http& s_client_type::GetResponse(void) { return this->response_msg_; }
void s_client_type::SetResponse(void) {
  this->response_msg_ = MakeResponseMessages(this);
}

size_t& s_client_type::GetMessageLength(void) { return this->msg_length; }
void s_client_type::SetMessageLength(size_t size) { this->msg_length = size; }
const t_stage& s_client_type::GetStage(void) { return this->stage_; }
void s_client_type::SetStage(t_stage val) {
  time_data_[1] = std::time(NULL);
  this->stage_ = val;
}

const t_error& s_client_type::GetErrorCode(void) { return this->status_code_; }
void s_client_type::SetErrorCode(t_error val) { this->status_code_ = val; }

const t_server& s_client_type::GetConfig(void) { return *this->config_ptr_; }
const s_server_type& s_client_type::GetParentServer(void) {
  return *(dynamic_cast<s_server_type*>(parent_ptr_));
}
const t_loc& s_client_type::GetLocationConfig(void) {
  return *this->loc_config_ptr_;
}
void s_client_type::SetConfigPtr(t_loc* ptr) { this->loc_config_ptr_ = ptr; }

s_work_type* s_client_type::GetChildWork(void) {
  return (dynamic_cast<s_work_type*>(data_ptr_));
}

bool s_client_type::GetCachePage(const std::string& uri, t_http& response) {
  t_server* rule = this->config_ptr_;
  std::string path;
  path += "./";
  path += uri;

  if (rule->static_pages_.find(path) == rule->static_pages_.end()) {
    return (false);
  }
  std::string temp_str(rule->static_pages_.find(path).operator->()->second);
  response.entity_length_ = temp_str.size();

  response.entity_ = new char[response.entity_length_ + 1];
  if (response.entity_ != NULL) {
    // TODO: error handling
  }
  temp_str.copy(response.entity_, response.entity_length_, 0);
  response.entity_[response.entity_length_] = '\0';
  temp_str.clear();
  return (true);
}

bool s_client_type::GetCacheError(t_error code, t_http& response) {
  t_server* rule = this->config_ptr_;

  std::ostringstream temp;
  temp << code;

  std::string err_key = temp.str();
  temp.clear();

  if (rule->error_pages_.find(err_key) == rule->error_pages_.end()) {
    return (false);
  }
  std::string temp_str(rule->error_pages_.find(err_key).operator->()->second);
  response.entity_length_ = temp_str.size();

  response.entity_ = new char[response.entity_length_ + 1];
  if (response.entity_ != NULL) {
    // TODO: error handling
  }
  temp_str.copy(response.entity_, response.entity_length_, 0);
  response.entity_[response.entity_length_] = '\0';
  temp_str.clear();
  return (true);
}

bool s_client_type::GetChunked(void) { return (this->GetStage() == GET_CHUNK); }
size_t s_client_type::GetChunkSize(void) { return this->sent_size_; }
bool s_client_type::IncreaseChunked(size_t sent_size) {
  size_t sent_unit = sent_size;
  sent_size_ += sent_unit;
  if (sent_size_ < this->response_msg_.entity_length_)
    return false;
  else
    return true;
}

std::time_t* s_client_type::GetTimeData(void) { return this->time_data_; }

void s_client_type::SetOriginURI(std::string path) { this->origin_uri_ = path; }
const std::string& s_client_type::GetOriginURI(void) {
  return this->origin_uri_;
}

void s_client_type::SetWorkFinishTime(void) {
  this->time_data_[1] = std::time(NULL);
}
void s_client_type::SetIP(const char* IP) { this->ip_.assign(IP); }
const std::string& s_client_type::GetIP(void) { return this->ip_; }

void s_client_type::PrintClientStatus(void) {
  char msg[40];
  const std::time_t* time = this->GetTimeData();
  std::strftime(msg, 30, "%d/%b/%Y:%H:%M:%S + 0900", std::localtime(&time[0]));

  std::cout << "==================================" << std::endl;
  std::cout << "Client Information" << std::endl;
  std::cout << "Accessed Host : ["
            << this->config_ptr_->main_config_.at("server_name") << "("
            << this->config_ptr_->port_ << ")]" << std::endl;
  std::cout << "ID : " << this->GetCookieId() << std::endl;
  std::cout << "Client IP : " << this->GetIP() << std::endl;
  std::cout << "FD : " << this->GetFD() << std::endl;
  std::cout << "ACCESS Time : " << msg << std::endl;
  std::cout << "Stage : " << this->GetStage() << std::endl;
  std::cout << "HTTP status : " << this->GetErrorCode() << std::endl;
  std::cout << "==================================" << std::endl;
}

void s_client_type::SendLogs(void) {
  std::string logging_data;
  const std::time_t* time = this->GetTimeData();
  char msg1[30];
  char msg2[30];
  std::strftime(msg1, 30, "%d/%b/%Y:%H:%M:%S + 0900", std::localtime(&time[0]));
  std::strftime(msg2, 30, "%d/%b/%Y:%H:%M:%S + 0900", std::localtime(&time[1]));
  s_logger_type* temp =
      &(static_cast<s_server_type*>(parent_ptr_))->GetLogger();
  std::vector<struct kevent> temp_event;

  if (this->GetErrorCode() == OK || this->GetErrorCode() == MOV_PERMAN) {
    ChangeEvents(temp_event, temp->GetLoggingFd(), EVFILT_WRITE, EV_ENABLE, 0,
                 NULL, &temp);
    logging_data.append(this->GetIP());
    logging_data.append(" [");
    logging_data.append(msg1);
    logging_data.append("] ");
    logging_data.append(this->response_msg_.init_line_.at("METHOD"));
    logging_data.append(" ");
    logging_data.append(this->origin_uri_);
    logging_data.append(" ");
    logging_data.append(this->response_msg_.init_line_.at("HTTP"));
    t_error temp = this->GetErrorCode();
    switch (temp) {
      case OK: {
        logging_data.append("200 Ok");
        break;
      }
      case MOV_PERMAN: {
        logging_data.append("308 Permanent Redirect");
        break;
      }
      default: {
        break;
      }
    }
    logging_data.append(" [Path : ");
    logging_data.append(this->response_msg_.init_line_.at("URI"));
    logging_data.append("]");
  } else {
    ChangeEvents(temp_event, temp->GetErrorFd(), EVFILT_WRITE, EV_ENABLE, 0,
                 NULL, &temp);
    logging_data.append("ERR : ");
    logging_data.append(this->GetIP());
    logging_data.append(" [");
    logging_data.append(msg1);
    logging_data.append("] ");
    logging_data.append(this->response_msg_.init_line_.at("METHOD"));
    logging_data.append(" ");
    logging_data.append(this->response_msg_.init_line_.at("URI"));
    logging_data.append(" ");
    logging_data.append(this->response_msg_.init_line_.at("HTTP"));
    logging_data.append(" [ERR_TIME ");
    logging_data.append(msg2);
    logging_data.append("] [");
    t_error temp = this->GetErrorCode();
    switch (temp) {
      case BAD_REQ: {
        logging_data.append("400 BadRequest");
        break;
      }
      case FORBID: {
        logging_data.append("403 Forbidden");
        break;
      }
      case NOT_FOUND: {
        logging_data.append("404 Not Found");
        break;
      }
      case NOT_IMPLE: {
        logging_data.append("501 Not Implemented");
        break;
      }
      case OLD_HTTP: {
        logging_data.append("505 HTTP version Not Supported");
        break;
      }
      case SYS_ERR: {
        logging_data.append("500 Interneal Server Error");
        break;
      }
      default: {
        break;
      }
    }
    logging_data.append("] [");
    std::ostringstream error_code;
    error_code << this->errno_;
    logging_data.append(error_code.str());
    logging_data.append(" : ");
    logging_data.append(strerror(errno_));
    logging_data.append("] [");
    logging_data.append("msg : ");
    logging_data.append(this->err_custom_);
    logging_data.append("]");
  }
  logging_data.append("\n");
  temp->GetData(logging_data);
  return;
}

/****************** Work Type ********************/

s_work_type::s_work_type(std::string& path, int fd, s_chore work_type,
                         t_http& response_msg)
    : s_base_type(fd),
      uri_(path),
      work_type_(work_type),
      response_msg_(response_msg) {}

s_work_type::~s_work_type() {}

void s_work_type::SetClientPtr(s_base_type* ptr) { this->client_ptr_ = ptr; }

s_base_type* s_work_type::GetClientPtr(void) { return this->client_ptr_; }

const std::string& s_work_type::GetUri(void) { return this->uri_; }

s_chore s_work_type::GetWorkType(void) { return this->work_type_; }

t_http& s_work_type::GetResponseMsg(void) { return this->response_msg_; }

void s_work_type::ChangeClientEvent(int16_t filter, uint16_t flags,
                                    uint16_t fflags, intptr_t data,
                                    void* udata) {
  int fd = this->client_ptr_->GetFD();
  std::vector<struct kevent> templist;
  ChangeEvents(templist, fd, filter, flags, fflags, data, udata);
}

t_stage s_work_type::GetClientStage(void) {
  s_client_type* my_mother = static_cast<s_client_type*>(this->client_ptr_);
  return (my_mother->GetStage());
}

void s_work_type::SetClientStage(t_stage val) {
  s_client_type* my_mother = static_cast<s_client_type*>(this->client_ptr_);
  my_mother->GetTimeData()[1] = std::time(NULL);
  my_mother->SetStage(val);
}
