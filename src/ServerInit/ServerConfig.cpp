#include "../../include/config.hpp"
#include "../../include/utils.hpp"

ServerConfig::ServerConfig(const char* confpath) : server_number_(0) {
  if (IsExistFile(confpath) == false) {
    PrintError(2, WEBSERV, CANNOTFOUND);
  }
  char* config_data = NULL;
  config_data = ReadASCI(confpath, -1);

  printf("File Size : %lu\n", strlen(config_data));
  ParssingServer(config_data);
  (void)server_number_;
}

// ServerConfig::~ServerConfig() {}

void ServerConfig::ParssingServer(const char* config_data) {
  unsigned long i = 0;
  std::string config_string;
  config_string.assign(config_data);

  while ((i = config_string.find(SER, i)) != std::string::npos) {
    if (CheckKeyWord(config_string, i, SER)) {
      t_ser_conf* temp = new t_ser_conf();
      this->server_number_++;
      this->server_list_.push_back(temp);
      SOUT << "position data : " << i << SEND;
    }
    i++;
  }
  SOUT << server_number_ << SEND;
}

bool ServerConfig::CheckKeyWord(const std::string& target, unsigned long pos,
                                const char* keyword) {
  int limit = strlen(keyword);
  int i = 0;
  while (pos != target.size()) {
    if (target.at(pos) == keyword[i]) {
      i++;
      if (i == limit) {
        if (IsWhiteSpace(target.at(pos + 1))) {
          return (true);
        } else
          return (false);
      }
    }
    pos++;
  }

  return (false);
}

// ssize_t ServerConfig::PrintServerConfig() {}
// t_ser_conf* ServerConfig::GetServer(int64_t server_number) {}
// t_ser_conf* ServerConfig::GetServer(const char* server_name) {}
// loc_list::iterator ServerConfig::GetServerLocation(int64_t server_number)
// {} loc_list::iterator ServerConfig::GetServerLocation(const char*
// server_name)
// {}
