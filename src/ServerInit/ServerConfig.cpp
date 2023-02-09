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
  PrintServerConfig();
  // TODO : 가변인자 방식으로 유효성 검사 진행하기
  // 1. server 전체 필수 인자 존재 여부
  // 2. 각 키별 값 상태 확인
  // 2.1 정수 키 -> 그 외의 값 들어가면 안됨
  // 2.2 특수 키 -> 특정 키워드, 크기 동일해야함
  // 2.3 html -> default_file 의 경우 html 만 지원 하므로, 그 외에는 아웃.
  // 2.4 cgi의 경우 cgi 이름이 곧 확장자 명 -> 즉 cgi 키 값과 이에 준하는 .py
  // 값의 파일 명이 지정되어 있어야함.
  // '/' (root) 로케이션은 반드시 존재해야함
}

// ServerConfig::~ServerConfig() {}

void ServerConfig::ParssingServer(const char* config_data) {
  pos_t i = 0;
  std::string config_string;
  config_string.assign(config_data);

  while ((i = config_string.find(SER, i)) != std::string::npos) {
    if (CheckKeyWord(config_string, i, SER)) {
      t_server* temp = new t_server();
      this->server_number_++;
      this->server_list_.push_back(temp);
      i = ParssingServerLine(config_string, i + 6);
    }
    i++;
  }
  if (server_number_ > 9) {
    PrintError(2, WEBSERV,
               "Multi-servers can be configured with less than 10 servers");
  }
}

pos_t ServerConfig::ParssingServerLine(std::string& config_string,
                                       pos_t init_pos) {
  pos_t i = init_pos;
  pos_t key_length = 0;
  pos_t value_length = 0;
  std::string temp_key;
  std::string temp_value;

  while (i < config_string.size()) {
    if (IsWhiteSpace(config_string.at(i))) {
      i++;
    } else if (config_string.at(i) == '{') {
      i++;
      while (IsWhiteSpace(config_string.at(i)) || config_string.at(i) == '\n') {
        i++;
      }
    } else if (isalnum(config_string.at(i))) {
      if (!CheckKeyWord(config_string, i, SER)) {
        pos_t value_loc = i;
        key_length = FindKeyLength(config_string, value_loc);
        temp_key = config_string.substr(i, key_length);
        if (temp_key == LOC) {
          i += key_length;
          while (IsWhiteSpace(config_string.at(i))) {
            i++;
          }
          value_loc = i;
          key_length = FindKeyLength(config_string, value_loc);
          temp_key = config_string.substr(i, key_length);

          t_loc* temp_loc = new t_loc();
          server_list_.at(server_number_ - 1)
              ->location_configs_.insert(
                  std::pair<std::string, t_loc*>(temp_key, temp_loc));
          while (config_string[i] != '{') {
            i++;
          }
          i++;
          while (config_string[i] != '}') {
            while (IsWhiteSpace(config_string.at(i))) {
              i++;
              if (config_string.at(i) == '#') {
                while (config_string.at(i) != '\n') {
                  i++;
                }
              } else if (isalnum(config_string.at(i))) {
                break;
              }
            }
            if (config_string[i] == '}') {
              PrintError(2, WEBSERV, "Config is empty");
            }
            value_loc = i;
            key_length = FindKeyLength(config_string, value_loc);
            temp_key = config_string.substr(i, key_length);
            value_length = FindValueLength(config_string, value_loc);
            temp_value = config_string.substr(value_loc, value_length);
            temp_loc->main_config_.insert(
                std::pair<std::string, std::string>(temp_key, temp_value));
            i = value_loc + value_length + 1;
            while (IsWhiteSpace(config_string.at(i))) {
              i++;
              if (config_string.at(i) == '#') {
                while (config_string.at(i) != '\n') {
                  i++;
                }
              } else if (isalnum(config_string.at(i))) {
                break;
              }
            }
          }
        } else {
          value_length = FindValueLength(config_string, value_loc);
          temp_value = config_string.substr(value_loc, value_length);
          server_list_.at(server_number_ - 1)
              ->main_config_.insert(
                  std::pair<std::string, std::string>(temp_key, temp_value));
          i = value_loc + value_length + 1;
        }

        while (IsWhiteSpace(config_string.at(i))) {
          i++;
        }

      } else {
        return (i - 1);
      }
    } else if (config_string.at(i) == '#') {
      while (config_string.at(i) != '\n') {
        i++;
      }
      i++;
    } else if (config_string.at(i) == '}') {
      i++;
    }
    init_pos = i;
  }
  i = init_pos;
  return (i);
}

bool ServerConfig::CheckKeyWord(const std::string& target, pos_t pos,
                                const char* keyword) {
  int limit = strlen(keyword);
  int i = 0;
  if (pos != target.size()) {
    while (target[pos] == keyword[i] && i < limit) {
      //   SOUT << target[pos] << " :  " << keyword[i] << SEND;
      i++;
      pos++;
      if (i == limit) {
        if (IsWhiteSpace(target.at(pos))) {
          return (true);
        } else {
          return (false);
        }
      }
    }
  }

  return (false);
}

ssize_t ServerConfig::PrintServerConfig() {
  int64_t i = 0;

  while (i < server_number_) {
    config_map::iterator it = server_list_[i]->main_config_.begin();
    SOUT << "[ " << BOLDBLUE << std::setw(16) << ""
         << "Server Number : " << i + 1 << RESET << std::setw(17) << " ]"
         << SEND;
    while (it != server_list_[i]->main_config_.end()) {
      SOUT << "[ " << GREEN << std::setw(15) << std::left
           << it.operator->()->first << RESET << " : ";
      SOUT << std::setw(30) << std::right << it.operator->()->second << " ]"
           << SEND;
      it++;
    }
    SOUT << SEND;
    std::map<std::string, t_loc*>::iterator lit =
        server_list_[i]->location_configs_.begin();
    while (lit != server_list_[i]->location_configs_.end()) {
      SOUT << "[ " << YELLOW << std::setw(15) << std::left << "● Location"
           << "   : " << std::setw(30) << std::right << lit.operator->()->first
           << RESET << " ]" << SEND;
      config_map::iterator temp =
          lit.operator->()->second->main_config_.begin();
      while (temp != lit.operator->()->second->main_config_.end()) {
        SOUT << "[ " << GREEN << std::setw(15) << std::left
             << temp.operator->()->first << RESET << " : ";
        SOUT << std::setw(30) << std::right << temp.operator->()->second << " ]"
             << SEND;
        temp++;
      }
      SOUT << SEND;
      lit++;
    }
    SOUT << SEND;
    i++;
  }
  return (0);
}
// t_server* ServerConfig::GetServer(int64_t server_number) {}
// t_server* ServerConfig::GetServer(const char* server_name) {}
// loc_list::iterator ServerConfig::GetServerLocation(int64_t server_number)
// {} loc_list::iterator ServerConfig::GetServerLocation(const char*
// server_name)
// {}
