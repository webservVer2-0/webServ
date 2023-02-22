#include "../../include/config.hpp"
#include "../../include/utils.hpp"

ServerConfig::ServerConfig(const char* confpath) : server_number_(0) {
  if (IsExistFile(confpath) == false) {
    PrintError(2, WEBSERV, CANNOTFOUND);
  }
  char* config_data = NULL;
  config_data = ReadASCI(confpath, -1);

  //   printf("File Size : %lu\n", strlen(config_data));
  ParssingServer(config_data);
  //   PrintServerConfig();
  ValidCheckMain();
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
          temp_loc->location_ = temp_key;
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
          if (temp_key == LISTEN) {
            server_list_.at(server_number_ - 1)->port_ = temp_key;
          }
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
      if (it.operator->()->second.size() < 30) {
        SOUT << "[ " << GREEN << std::setw(15) << std::left
             << it.operator->()->first << RESET << " : ";
        SOUT << std::setw(30) << std::right << it.operator->()->second << " ]"
             << SEND;
      } else {
        pos_t limit = it.operator->()->second.size();
        pos_t cnt = 0;
        pos_t pos = 0;
        pos_t skip_white_space = 0;
        std::string temp = it.operator->()->second;
        while (cnt < limit) {
          SOUT << "[ " << GREEN << std::setw(15) << std::left
               << it.operator->()->first << RESET << " : ";
          skip_white_space = 0;
          while (cnt < limit) {
            if (temp.at(cnt) == ' ') {
              skip_white_space++;
              if (skip_white_space == 2) {
                break;
              }
            }
            if (cnt == limit) {
              break;
            }
            cnt++;
          }
          SOUT << std::setw(30) << std::right << temp.substr(pos, cnt) << " ]"
               << SEND;
          cnt++;
          pos = cnt;
        }
      }
      it++;
    }
    SOUT << SEND;
    std::map<std::string, t_loc*>::iterator lit =
        server_list_[i]->location_configs_.begin();
    while (lit != server_list_[i]->location_configs_.end()) {
      SOUT << "[ " << YELLOW << std::setw(15) << std::left << "● Location"
           << "   : " << std::setw(30) << std::right
           << lit.operator->()->second->location_ << RESET << " ]" << SEND;
      config_map::iterator temp =
          lit.operator->()->second->main_config_.begin();
      while (temp != lit.operator->()->second->main_config_.end()) {
        if (temp.operator->()->second.size() <= 30) {
          SOUT << "[ " << GREEN << std::setw(15) << std::left
               << temp.operator->()->first << RESET << " : ";
          SOUT << std::setw(30) << std::right << temp.operator->()->second
               << " ]" << SEND;
        } else {
          pos_t limit = it.operator->()->second.size();
          pos_t cnt = 0;
          pos_t pos = 0;
          pos_t skip_white_space = 0;
          std::string temp = it.operator->()->second;
          while (cnt < limit) {
            SOUT << "[ " << GREEN << std::setw(15) << std::left
                 << it.operator->()->first << RESET << " : ";
            skip_white_space = 0;
            while (cnt < limit) {
              if (temp.at(cnt) == ' ') {
                skip_white_space++;
                if (skip_white_space == 2) {
                  break;
                }
              }
              if (cnt == limit) {
                break;
              }
              cnt++;
            }
            SOUT << std::setw(30) << std::right << temp.substr(pos, cnt)
                 << SEND;
            cnt++;
            pos = cnt;
          }
        }
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

void ServerConfig::ServerConfig::ValidCheckMain(void) {
  int64_t i = 0;
  conf_iterator* error_value;

  while (i < server_number_) {
    if (!ValidCheckServer(i, *error_value)) {
      ;
    }
    std::map<std::string, t_loc*> temp_loc =
        server_list_.at(i)->location_configs_;
    std::map<std::string, t_loc*>::iterator it = temp_loc.begin();
    while (it != temp_loc.end()) {
      if (!ValidCheckLocation(i, it.operator->()->first, *error_value)) {
        ;
      }
      it++;
    }
    i++;
    error_value = NULL;
  }
}

bool ServerConfig::ValidConfigNumber(conf_iterator& target, char* standard,
                                     conf_iterator& error_log) {
  std::string temp = target.operator->()->second;
  int i = 0;
  int size = temp.size();
  while (i < size) {
    if (!isnumber(temp.at(i))) {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
    i++;
  }
  int number_value = atoi(temp.c_str());
  if (number_value <= 0 || number_value > INT_MAX) {
    PrintError(5, WEBSERV, "Server", standard,
               "value has a problem : ", temp.c_str());
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigFilePath(conf_iterator& target,
                                       conf_iterator& error_log) {
  struct stat s;
  std::string temp_path;
  temp_path.append("./");
  temp_path.append(target.operator->()->second);
  if (stat(temp_path.c_str(), &s) == 0) {
    if ((s.st_mode & S_IFDIR) == false) {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
  } else {
    PrintError(4, WEBSERV, "Location Config Error",
               target.operator->()->first.c_str(),
               target.operator->()->second.c_str());
    return (false);
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigFile(conf_iterator& target,
                                   conf_iterator& error_log) {
  struct stat s;
  std::string temp_path;
  temp_path.append("./");
  temp_path.append(target.operator->()->second);
  if (stat(temp_path.c_str(), &s) == 0) {
    if ((s.st_mode & S_IFDIR) == true) {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
  } else {
    PrintError(4, WEBSERV, "Location Config Error",
               target.operator->()->first.c_str(),
               target.operator->()->second.c_str());
    return (false);
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigCGIFile(conf_iterator& target,
                                      conf_iterator& error_log,
                                      t_loc& location) {
  struct stat s;
  std::string temp_path;
  temp_path.append("./");
  temp_path.append(location.main_config_.at(std::string(ROOT)));
  temp_path.append("/");
  temp_path.append(target.operator->()->second);
  if (stat(temp_path.c_str(), &s) == 0) {
    if ((s.st_mode & S_IFDIR) == true) {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
  } else {
    PrintError(4, WEBSERV, "Location Config Error",
               target.operator->()->first.c_str(),
               target.operator->()->second.c_str());
    return (false);
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigStr(conf_iterator& target,
                                  conf_iterator& error_log) {
  std::string temp = target.operator->()->second;
  int i = 0;
  int size = temp.size();
  while (i < size) {
    if (!isalnum(temp.at(i)) && !(temp.at(i) == '_')) {
      if (target.operator->()->first.compare(METHOD) == 0 &&
          (isalnum(temp.at(i)) || temp.at(i) == ' ')) {
        i++;
        continue;
      } else {
        PrintError(4, WEBSERV, "Location Config Error",
                   target.operator->()->first.c_str(),
                   target.operator->()->second.c_str());
        return (false);
      }
    }
    i++;
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigHTML(conf_iterator& target,
                                   conf_iterator& error_log) {
  std::string temp = target.operator->()->second;
  int i = 0;
  int limit = temp.size();
  while (i < limit) {
    if (isalnum(temp.at(i))) {
      i++;
    } else if (temp.at(i) == '.') {
      if (temp.find("html", i) != std::string::npos) {
        if (i + 5 == limit) {
          return (true);
        }
      } else {
        PrintError(4, WEBSERV, "Location Config Error",
                   target.operator->()->first.c_str(),
                   target.operator->()->second.c_str());
        return (false);
      }
    } else {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigAutoindex(conf_iterator& target,
                                        conf_iterator& error_log,
                                        int server_number) {
  std::string temp = target.operator->()->second;
  server_list_.at(server_number)->index_mode_ = autodef;
  if (temp.size() == 2) {
    if (temp == "on") {
      server_list_.at(server_number)->index_mode_ = on;
    }
  } else if (temp.size() == 3) {
    if (temp == "off") {
      server_list_.at(server_number)->index_mode_ = off;
    }
  } else {
    PrintError(4, WEBSERV, "Location Config Error",
               target.operator->()->first.c_str(),
               target.operator->()->second.c_str());
    return (false);
  }
  if (server_list_.at(server_number)->index_mode_ == autodef) {
    PrintError(4, WEBSERV, "Location Config Error",
               target.operator->()->first.c_str(),
               target.operator->()->second.c_str());
    return (false);
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigAutoindexLocation(conf_iterator& target,
                                                conf_iterator& error_log,
                                                t_loc& location) {
  std::string temp = target.operator->()->second;
  if (temp.size() == 2) {
    if (temp == "on") {
      location.index_mode_ = on;
    }
  } else if (temp.size() == 3) {
    if (temp == "off") {
      location.index_mode_ = off;
    }
  } else {
    PrintError(4, WEBSERV, "Location Config Error",
               target.operator->()->first.c_str(),
               target.operator->()->second.c_str());
    return (false);
  }
  if (location.index_mode_ == autodef) {
    PrintError(4, WEBSERV, "Location Config Error",
               target.operator->()->first.c_str(),
               target.operator->()->second.c_str());
    return (false);
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidConfigError(conf_iterator& target,
                                    conf_iterator& error_log) {
  std::string temp = target.operator->()->second;
  int i = 0;
  int limit = temp.size();
  std::string value;

  while (i < limit) {
    if (isnumber(temp.at(i))) {
      while (isalnum(temp.at(i))) {
        value.push_back(temp.at(i));
        i++;
      }
      if (temp.at(i) == ' ') {
        int errnum = atoi(value.c_str());
        if (errnum < 100 || errnum >= 600) {
          PrintError(4, WEBSERV, "Location Config Error",
                     target.operator->()->first.c_str(),
                     target.operator->()->second.c_str());
          return (false);
        }
      } else {
        PrintError(4, WEBSERV, "Location Config Error",
                   target.operator->()->first.c_str(),
                   target.operator->()->second.c_str());
        return (false);
      }
      while (IsWhiteSpace(temp.at(i))) {
        i++;
      }
      int pos = i;
      while (!IsWhiteSpace(temp.at(i))) {
        i++;
        if (i == limit) {
          break;
        }
      }
      value.clear();
      value = temp.substr(pos, i - pos);
      struct stat s;
      std::string temp_path;
      temp_path.append("./");
      temp_path.append(value);
      if (stat(temp_path.c_str(), &s) == 0) {
        if ((s.st_mode & S_IFDIR) == true) {
          PrintError(4, WEBSERV, "Location Config Error",
                     target.operator->()->first.c_str(),
                     target.operator->()->second.c_str());
          return (false);
        }
      } else {
        PrintError(4, WEBSERV, "Location Config Error",
                   target.operator->()->first.c_str(),
                   target.operator->()->second.c_str());
        return (false);
      }
      temp_path.clear();
      value.clear();
      while (i < limit && IsWhiteSpace(temp.at(i))) {
        i++;
      }
    } else {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
  }
  (void)error_log;
  return (true);
}

bool ServerConfig::ValidCheckServer(int server_number,
                                    conf_iterator& error_log) {
  conf_iterator target = server_list_.at(server_number)->main_config_.begin();
  conf_iterator end = server_list_.at(server_number)->main_config_.end();
  std::string temp;
  while (target != end) {
    temp.assign(target.operator->()->first);
    if (0 == temp.compare(LISTEN)) {
      char* listen = strdup(LISTEN);
      if (!ValidConfigNumber(target, listen, error_log)) {
        return (false);
      }
      delete[] listen;
    } else if (0 == temp.compare(BODY)) {
      char* body = strdup(BODY);
      if (!ValidConfigNumber(target, body, error_log)) {
        return (false);
      }
      delete[] body;
    } else if (0 == temp.compare(MAXCON)) {
      char* maxcon = strdup(MAXCON);
      if (!ValidConfigNumber(target, maxcon, error_log)) {
        return (false);
      }
      delete[] maxcon;
    } else if (0 == temp.compare(ROOT)) {
      if (!ValidConfigFilePath(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(DEFFILE)) {
      if (!ValidConfigHTML(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(UPLOAD)) {
      if (!ValidConfigFilePath(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(ACCLOG)) {
      if (!ValidConfigFile(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(ERRLOG)) {
      if (!ValidConfigFile(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(SERNAME)) {
      if (!ValidConfigStr(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(TIMEOUT)) {
      char* timeout = strdup(TIMEOUT);
      if (!ValidConfigNumber(target, timeout, error_log)) {
        return (false);
      }
      delete[] timeout;
    } else if (0 == temp.compare(AUTOINDEX)) {
      if (!ValidConfigAutoindex(target, error_log, server_number)) {
        return (false);
      }
    } else if (0 == temp.compare(METHOD)) {
      if (!ValidConfigStr(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(ERR)) {
      if (!ValidConfigError(target, error_log)) {
        return (false);
      }
    } else {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
    temp.clear();
    target++;
  }
  //   SOUT << "Server Valid Check Successed!" << SEND;
  return (true);
}

bool ServerConfig::ValidCheckLocation(int server_number,
                                      std::string location_name,
                                      conf_iterator& error_log) {
  conf_iterator target = server_list_.at(server_number)
                             ->location_configs_.at(location_name)
                             ->main_config_.begin();
  conf_iterator end = server_list_.at(server_number)
                          ->location_configs_.at(location_name)
                          ->main_config_.end();
  t_loc* target_location =
      server_list_.at(server_number)->location_configs_.at(location_name);
  for (int i = 0; i < 3; i++) {
    target_location->loc_type_[0] = T_NULL;
  }
  target_location->index_mode_ = autodef;
  std::string temp;
  while (target != end) {
    temp.assign(target.operator->()->first);
    if (temp.compare(ROOT) == 0) {
      if (!ValidConfigFilePath(target, error_log)) {
        return (false);
      }
      target_location->loc_type_[0] = T_ROOT;
    } else if (temp.compare(METHOD) == 0) {
      if (!ValidConfigStr(target, error_log)) {
        return (false);
      }
    } else if (temp.compare(DEFFILE) == 0) {
      if (!ValidConfigHTML(target, error_log)) {
        return (false);
      }
    } else if (temp.compare(AUTOINDEX) == 0) {
      if (!ValidConfigAutoindexLocation(target, error_log, *target_location)) {
        return (false);
      }
    } else if (temp.compare(REDIR) == 0) {
      if (!ValidConfigFilePath(target, error_log)) {
        return (false);
      }
      target_location->loc_type_[1] = T_REDIR;
    } else if (temp.compare(CGIFILE) == 0) {
      if (!ValidConfigCGIFile(target, error_log, *target_location)) {
        return (false);
      }
      target_location->loc_type_[2] = T_CGI;
    } else if (temp.compare(LOC) == 0) {
      if (!ValidConfigStr(target, error_log)) {
        return (false);
      }
    } else if (temp.compare(ERR) == 0) {
      if (!ValidConfigError(target, error_log)) {
        return (false);
      }
    } else if (0 == temp.compare(UPLOAD)) {
      if (!ValidConfigFilePath(target, error_log)) {
        return (false);
      }
    } else {
      PrintError(4, WEBSERV, "Location Config Error",
                 target.operator->()->first.c_str(),
                 target.operator->()->second.c_str());
      return (false);
    }
    temp.clear();
    target++;
  }
  //   SOUT << "location Validation check is Finished : "
  //        << target_location->location_ << SEND;
  return (true);
}

// t_server* ServerConfig::GetServer(int64_t server_number) {}
// t_server* ServerConfig::GetServer(const char* server_name) {}
// loc_list::iterator ServerConfig::GetServerLocation(int64_t server_number)
// {} loc_list::iterator ServerConfig::GetServerLocation(const char*
// server_name)
// {}
