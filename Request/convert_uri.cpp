#include "../include/webserv.hpp"
/**
 * @brief 테스트용 출력함수
 *
 * @param vec
 */
void print_vector_path(std::vector<std::string> &vec) {
  std::cout << ".";
  for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();
       ++it) {
    std::cout << "/" + *it;
  }
  std::cout << std::endl;
}

/**
 * @brief 토큰화된 string을 제대로된 경로로 만들어주는 함수
 *
 * @param vec 토큰화된 string 벡터
 * @return std::string
 */
std::string make_uri_path(std::vector<std::string> &vec) {

  std::string ret;

  ret.append(".");
  for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end();
       ++it) {
    ret.append("/" + *it);
  }
}

t_error convert_uri(std::string rq_uri,
                    std::map<std::string, t_loc *> location_config,
                    s_client_type &client) {
  std::vector<std::string> rq_path(1);
  size_t pos;
  std::map<std::string, t_loc *>::iterator loc_it;

  std::string token = rq_uri;
  while ((pos = rq_uri.find('/')) != std::string::npos) {
    token = rq_uri.substr(0, pos);
    if (token != "" && token != ".") rq_path.push_back(token);
    rq_uri.erase(0, pos + 1);
  }
  if (rq_uri != "" && rq_uri != ".") rq_path.push_back(rq_uri);
  loc_it = location_config.find(("/" + rq_path.front()));
  rq_path.front() = (*loc_it).second->main_config_[ROOT];
  if (rq_path.size() == 1)
    rq_path.push_back((*loc_it).second->main_config_[DEFFILE]);

  if ((*loc_it).second->main_config_[METHOD].find(client.GetRequest().init_line_["METHOD"]) == std::string().npos)
  {
    return FORBID;
  }
  client.SetConfigPtr((*loc_it).second);
  client.GetRequest().init_line_["URI"] = make_uri_path(rq_path);
  print_vector_path(rq_path);
  return OK;
}