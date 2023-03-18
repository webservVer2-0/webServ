#include "../../include/datas.hpp"

s_logger_type::s_logger_type() : s_base_type(-1) { this->SetType(LOGGER); }

s_logger_type::~s_logger_type() {
  logs_.clear();
  ServerConfig::ChangeEvents(logging_fd_, EVFILT_WRITE, EV_DELETE, 0, NULL,
                             NULL);
  close(logging_fd_);
  close(error_fd_);
}

void s_logger_type::GetData(std::string log) {
  std::string* log_ = new std::string(log);
  logs_.push_back(log_);
  std::vector<struct kevent> temp;
  ServerConfig::ChangeEvents(this->logging_fd_, EVFILT_WRITE, EV_ENABLE, 0,
                             NULL, this);
}
void s_logger_type::PushData(void) {
  size_t limit = logs_.size();

  if (limit == 0) return;
  std::cout << "Limit : " << limit << std::endl;
  for (size_t i = 0; i < limit; i++) {
    if (logs_[i]->at(0) == 'E') {
      write(error_fd_, logs_.at(i)->c_str(), logs_.at(i)->size());
    } else {
      write(logging_fd_, logs_.at(i)->c_str(), logs_.at(i)->size());
    }
    delete logs_.at(i);
    logs_.clear();
  }
  std::vector<struct kevent> temp;
  ServerConfig::ChangeEvents(this->logging_fd_, EVFILT_WRITE, EV_DISABLE, 0,
                             NULL, this);
}

void s_logger_type::SetFDs(int log_fd, int err_fd) {
  this->logging_fd_ = log_fd;
  this->error_fd_ = err_fd;
}

int s_logger_type::GetLoggingFd(void) { return this->logging_fd_; }
int s_logger_type::GetErrorFd(void) { return this->error_fd_; }

void s_logger_type::PrintLogger(void) {
  std::cout << "==================================" << std::endl;
  std::cout << "Logger Information" << std::endl;
  std::cout << "Access Logger Fd : " << this->logging_fd_ << std::endl;
  std::cout << "Error Logger Fd : " << this->error_fd_ << std::endl;
  std::cout << "Logging Storage : " << this->logs_.size() << std::endl;
  std::cout << "==================================" << std::endl;
}
