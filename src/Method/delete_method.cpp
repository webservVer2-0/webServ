#include "../../include/webserv.hpp"

/*
GET ~~~/delete?delete_target=137_file.png HTTP/1.1
GET upload_path/137_file.png HTTP/1.1
enitity
entity_length_
MiME
*/

/**
 * @brief Request에서 변경한 URI에서 "?delete_target=" 문자열을
 * 제거한 문자열(삭제 파일 경로)을 생성하는 함수입니다.
 *
 * @param uri_w_query Query("?delete_target=")가 들어있는 URI
 * @return std::string Query가 삭제된 URI string (삭제 파일 경로)
 * @note POST에 있는 파일 경로 함수와 다르게 이 함수는 인자로 std::string&가
 * 아닌 std::string을 받습니다. 그 결과 client 내부의 ConvertedURI는
 * 변경되지 않습니다.
 */
std::string EraseQueryFromUri(std::string uri_w_query) {
  size_t query_start = uri_w_query.find("?");
  size_t erase_len = (uri_w_query.find("=") - query_start) + 1;

  return uri_w_query.erase(query_start, erase_len);
}

void ClientDelete(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);

  std::string file_path = EraseQueryFromUri(client->GetConvertedURI());
  const char* path_char = file_path.c_str();

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
