#include "../../include/webserv.hpp"

typedef struct s_elem {
  std::string _init_line;
  std::string _header_line;
  char* _entity;
  e_method _method;
  bool _error;
  bool _entity_exist;
  size_t _content_length;
  size_t _header_end;
} t_elem;

/**
 * @brief 요청받은 uri를 실제 uri로 변환
 *
 * @param elem
 * @param http
 * @param location_config
 * @return 미정
 *
 */
typedef enum t_method {
  GET,
  POST,
  DELETE,
} e_method;
// TO-DO:
//  정확하게 일치하는지?

// 루트인지?

// 디폴트파일 사용 여부

// 치환후 ok

// 메소드가 맞는지?

// 진짜 있는 파일인지?

t_error convert_uri(std::string rq_uri,
                    std::map<std::string, t_loc*> location_config) {
  std::vector<std::string> rq_path;
  size_t pos;
  std::string token;

  if (rq_uri.at(0) != '/') return BAD_REQ;
  rq_uri = rq_uri.substr(1);
  while ((pos = rq_uri.find('/')) != std::string::npos) {
    token = rq_uri.substr(0, pos);
    rq_path.push_back(token);
    rq_uri.erase(0, pos + 1);
  }
  rq_path.push_back(rq_uri);

  std::cout << "here " << *rq_path.begin() << std::endl;
  std::map<std::string, t_loc*>::iterator it =
      location_config.find(("/" + *rq_path.begin()));
  if (it != location_config.end()) {
    std::cout << "found" << std::endl;
    *rq_path.begin() = (*it).second->main_config_[ROOT];
  } else {
    it = location_config.find("/");
    *rq_path.begin() = location_config["/"]->main_config_[ROOT];
  }
  struct stat s;
  std::string tmp_path(".");
  for (std::vector<std::string>::iterator tmp_it = rq_path.begin();
       tmp_it != rq_path.end(); ++tmp_it) {
    tmp_path.append("/");
    tmp_path.append(*tmp_it);
  }
  if (stat(tmp_path.c_str(), &s) == 0 && (s.st_mode & S_IFDIR)) {
    rq_path.back() = (*it).second->main_config_[DEFFILE];
  }

  std::cout << "vector size : " << rq_path.size() << std::endl;
  for (std::vector<std::string>::iterator tmp_it = rq_path.begin();
       tmp_it != rq_path.end(); ++tmp_it) {
    std::cout << "/" << *tmp_it;
  }
  std::cout << std::endl;

  return OK;
}