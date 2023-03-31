#include "../../include/datas.hpp"

s_logger_type::s_logger_type() : s_base_type(-1) {
  this->SetType(LOGGER);
  data_que_ = 0;
}

s_logger_type::~s_logger_type() {
  logs_.clear();
  ServerConfig::ChangeEvents(this->GetFD(), EVFILT_WRITE, EV_DELETE, 0, NULL,
                             NULL);
  close(this->GetFD());
}

void s_logger_type::GetData(std::string log) {
  data_que_ += 1;
  logs_.push_back(log);
  ServerConfig::ChangeEvents(this->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, NULL,
                             this);
}
void s_logger_type::PushData(void) {
  if (data_que_ == 0) {
    return;
  }
  for (size_t i = 0; i < data_que_; i++) {
    write(GetFD(), logs_.at(i).c_str(), logs_.at(i).size());
  }
  logs_.clear();
  ServerConfig::ChangeEvents(this->GetFD(), EVFILT_WRITE, EV_DISABLE, 0, NULL,
                             this);
  data_que_ = 0;
}

t_log_type s_logger_type::GetLabel(void) { return this->log_type_; }

void s_logger_type::SetLabel(s_log_type type) { this->log_type_ = type; }

void s_logger_type::PrintLogger(void) {
  std::cout << "==================================" << std::endl;
  std::cout << "Logger Information" << std::endl;
  std::cout << "Logger Fd : " << this->GetFD() << std::endl;
  std::cout << "Logger type : ";
  if (this->GetLabel() == logger) {
    std::cout << "Logger Type" << std::endl;
  } else
    std::cout << "Error Logger Type" << std::endl;
  std::cout << "Logging Storage : " << this->logs_.size() << "(개)"
            << std::endl;
  std::cout << "==================================" << std::endl;
}
