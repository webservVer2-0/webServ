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
  pos_t i = 0;
  std::string config_string;
  config_string.assign(config_data);

  while ((i = config_string.find(SER, i)) != std::string::npos) {
    keywords_type server_keywords;
    keywords_type location_keywords;

    InitKeywords(server_keywords, 0);
    InitKeywords(location_keywords, 1);

    if (CheckKeyWord(config_string, i, SER)) {
      t_ser_conf* temp = new t_ser_conf();
      this->server_number_++;
      this->server_list_.push_back(temp);
      // TODO : 서버 별 config 읽기
      i = ParssingServerLine(server_keywords, location_keywords, config_string,
                             i + 6);
      // TODO : location 발견 시 location 읽기 진행

      SOUT << "position data : " << i << SEND;
    }
    i++;
  }
  SOUT << server_number_ << SEND;
}

pos_t ServerConfig::ParssingServerLine(keywords_type server_list,
                                       keywords_type location_list,
                                       std::string& config_string,
                                       pos_t init_pos) {
  PrintLine(config_string, init_pos);

  pos_t i = init_pos;
  pos_t key_length = 0;
  std::string temp_key;
  std::string temp_value;

  while (!CheckKeyWord(config_string, init_pos, SER)) {
    if (IsWhiteSpace(config_string.at(i))) {
      i++;
    } else if (config_string.at(i) == '{') {
      i++;
      while (IsWhiteSpace(config_string.at(i)) || config_string.at(i) == '\n') {
        i++;
      }
    } else if (isalpha(config_string.at(i))) {
      // TODO : line 처리
      while (config_string.at(i) != '\n') {
        pos_t value_loc = i;
        key_length = StrLenNewLine(config_string, value_loc);
        // i = 현재 key의 첫글자 위치
        // value_loc = 현재 key의 마지막 글자 위치
        temp_key = config_string.substr(i, key_length);
        string_list* temp = &server_list.at(key_length);
        string_list::iterator it = temp->begin();
        while (it != (*temp).end()) {
          if (it.base() == temp_key) }
        // 1. key value 찾기(몇글자?) -> server_list 중 같은 key 인 곳에서
        // 찾아옴
        // 2. 해당 server key list 중 해당하는에를 찾음, e_server 번호로 값을
        // 읽어 하부 스트링을 전달한다.
      }
      // TODO : Location 나왔을시 특수 처리
      // TODO : # 이 나오면 해당 라인 무시 처리 해야함
    } else if (config_string.at(i) == '#') {
      // TODO : 주석 처리
    } else if (config_string.at(i) == '}') {
    }
  }

  return (i);
}

void ServerConfig::InitKeywords(keywords_type& list, int code) {
  pos_t i = 0;
  char** keywords;

  if (code == 0) {
    keywords = new char*[14];
    keywords[13] = NULL;
    keywords[0] = strdup(LISTEN);
    keywords[1] = strdup(BODY);
    keywords[2] = strdup(MAXCON);
    keywords[3] = strdup(ROOT);
    keywords[4] = strdup(DEFFILE);
    keywords[5] = strdup(UPLOAD);
    keywords[6] = strdup(ACCLOG);
    keywords[7] = strdup(ERRLOG);
    keywords[8] = strdup(SERNAME);
    keywords[9] = strdup(TIMEOUT);
    keywords[10] = strdup(AUTOINDEX);
    keywords[11] = strdup(METHOD);
    keywords[12] = strdup(ERR);
  } else {
    keywords = new char*[9];
    keywords[8] = NULL;
    keywords[0] = strdup(LOCNAME);
    keywords[1] = strdup(CGIFILE);
    keywords[2] = strdup(ERR);
    keywords[3] = strdup(METHOD);
    keywords[4] = strdup(AUTOINDEX);
    keywords[5] = strdup(DEFFILE);
    keywords[6] = strdup(MAXCON);
    keywords[7] = strdup(REDIR);
  }

  while (keywords[i]) {
    if (list.find(strlen(keywords[i])) != list.end()) {
      keywords_type::iterator target_list = list.find(strlen(keywords[i]));
      target_list.operator->()->second.push_back(std::string(keywords[i]));
    } else {
      string_list temp_array;
      std::string temp_string(keywords[i]);
      temp_array.push_back(temp_string);
      list.insert(std::pair<int, string_list>(temp_string.size(), temp_array));
    }
    i++;
  }

  i = 0;
  while (keywords[i]) {
    delete[] keywords[i];
    i++;
  }
  delete[] keywords;
}

bool ServerConfig::CheckKeyWord(const std::string& target, pos_t pos,
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
