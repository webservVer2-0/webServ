#include "../../include/webserv.hpp"

/*
GET ~~~/delete?delete_target=137_file.png HTTP/1.1
GET upload_path/?delete_target=137_file.png HTTP/1.1
enitity
entity_length_
MiME
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

  MakeDeletePage(client, client->GetResponse(), file_path);
  ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                             client);
  ServerConfig::ChangeEvents(client->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                             client);
  client->SetMimeType(file_path);
  client->SetErrorCode(OK);
  client->SetStage(DELETE_FIN);
  return;
}
