#include "../../include/webserv.hpp"

/**
 * @brief Request entity로 들어오는 "?delete_target=..." 쿼리문에서
 * delete_target을 분리한 후, upload_path를 붙여 지울 파일의 경로를 std::string
 * 형태로 생성하는 함수입니다.
 *
 * @param client s_client_type의 udata
 * @return std::string 실제 지울 delete_target이 존재하는 파일 경로
 */
std::string MakeDeleteTargetPath(s_client_type* client) {
  char* raw_entity = client->GetRequest().entity_;
  size_t entity_len = client->GetRequest().entity_length_;

  char* target_start = strchr(raw_entity, '=') + 1;
  size_t target_len = (raw_entity + entity_len) - target_start;
  char* delete_target = new char[target_len + 1];

  memmove(delete_target, target_start, target_len);
  delete_target[target_len] = '\0';

  std::string delete_path(delete_target);
  delete[] delete_target;
  delete_path.insert(0, "/");
  delete_path.insert(
      0, client->GetConfig().main_config_.find("upload_path")->second);
  return delete_path;
}

void ClientDelete(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);

  std::string delete_path = MakeDeleteTargetPath(client);
  const char* path_char = delete_path.c_str();

  unlink(path_char);

  std::string upload_path =
      client->GetConfig().main_config_.find("upload_path")->second;

  MakeDeletePage(client, client->GetResponse(), upload_path);
  ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                             client);
  ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                             client);

  client->SetMimeType(upload_path.append("/delete.html"));
  client->SetErrorCode(OK);
  client->SetStage(DELETE_FIN);
  return;
}
