#include "../../include/datas.hpp"

s_logger_type::s_logger_type() : s_base_type(-1) { this->SetType(LOGGER); }

s_logger_type::~s_logger_type() {
  logs_.clear();
  std::vector<struct kevent> temp;
  ChangeEvents(temp, logging_fd_, EVFILT_WRITE, EV_DELETE, 0, NULL, NULL);
  ChangeEvents(temp, error_fd_, EVFILT_WRITE, EV_DELETE, 0, NULL, NULL);
  close(logging_fd_);
  close(error_fd_);
}

void s_logger_type::GetData(std::string log) {
  logs_.push_back(log);
  std::vector<struct kevent> temp;
  ChangeEvents(temp, this->error_fd_, EVFILT_WRITE, EV_ENABLE, 0, NULL, this);
  ChangeEvents(temp, this->logging_fd_, EVFILT_WRITE, EV_ENABLE, 0, NULL, this);
}
void s_logger_type::PushData(void) {
  size_t limit = logs_.size();
  if (limit == 0) return;
  for (size_t i = 0; i < limit; i++) {
    if (logs_.at(i).find("ERR") == 0) {
      write(error_fd_, logs_.at(i).c_str(), logs_.at(i).size());
      write(error_fd_, "\n", 1);
    } else {
      write(logging_fd_, logs_.at(i).c_str(), logs_.at(i).size());
      write(logging_fd_, "\n", 1);
    }
  }
  logs_.clear();
  std::vector<struct kevent> temp;
  ChangeEvents(temp, this->error_fd_, EVFILT_WRITE, EV_DISABLE, 0, NULL, this);
  ChangeEvents(temp, this->logging_fd_, EVFILT_WRITE, EV_DISABLE, 0, NULL,
               this);
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
