#include "../../include/utils.hpp"
#include "../../include/webserv.hpp"

void MakeDeletePage(s_client_type* client, t_http& response,
                    std::string directory_path) {
  MakeHead(response);
  MakeDeleteBody(client, directory_path, response);
  MakeFooter(response);
}

void MakeDeleteBody(s_client_type* client, std::string directory_path,
                    t_http& response) {
  std::string entity;
  std::string pre_entity;

  for (size_t i = 0; i < response.entity_length_; i += 2) {
    pre_entity.push_back(response.entity_[i]);
    if (i + 1 < response.entity_length_) {
      pre_entity.push_back(response.entity_[i + 1]);
    }
  }
  delete[] response.entity_;

  pre_entity.append("	<h1>");
  pre_entity.append("DELETE_PAGE");
  pre_entity.append("</h1>\n");

  pre_entity.append("<div class=\"grid-container\">");

  DIR* dir;
  struct dirent* ent;
  std::map<std::string, std::string> file_mapping;
  dir = opendir(directory_path.c_str());
  if (dir != NULL) {
    ent = readdir(dir);
    while (ent != NULL) {
      if (IsDirectory(ent)) {
        ;
      } else if (IsFile(ent) &&
                 (std::string(ent->d_name).find(client->GetCookieId()) == 0)) {
        entity.append("  <div class=\"grid-item\">");
        entity.append(
            "<img src = \"./asset/file.png\" alt = \"아이콘\" width=\"32\" "
            "height=\"32\">");
        entity.append("    <a href=\"");
        entity.append("/download/");
        entity.append(std::string(ent->d_name));
        entity.append("\" style=\"margin-left:10px;\" download>");

        std::string temp =
            MakeNameWithoutID(client->GetCookieId(), std::string(ent->d_name));
        entity.append(temp);
        entity.append("</a>");
        entity.append(
            "<form method=\"DELETE\" action=\"/delete\" "
            "accept-charset=\"ASCII\"><input type =\"hidden\" "
            "name=\"delete_target\" value=\"");
        entity.append(ent->d_name);
        entity.append("\">");
        entity.append(
            "<button type=\"submit\"><img src=\"./asset/bin.png\" "
            "style=\"margin-left: 10px; margin-top: 5px;\" "
            "width=\"16px;\"></button></form></div>");
        file_mapping.insert(std::make_pair(std::string(ent->d_name),
                                           std::string(entity.c_str())));
      }
      entity.clear();
      ent = readdir(dir);
    }
  }
  entity.append(pre_entity);
  std::map<std::string, std::string>::iterator it = file_mapping.begin();
  std::map<std::string, std::string>::iterator end = file_mapping.end();
  while (it != end) {
    entity.append(it->second);
    it++;
  }
  entity.append("</div>");
  closedir(dir);
  response.entity_ = new char[entity.size()];
  response.entity_length_ = entity.size();
  for (size_t i = 0; i < response.entity_length_; i += 2) {
    response.entity_[i] = entity[i];
    if (i + 1 < response.entity_length_) {
      response.entity_[i + 1] = entity[i + 1];
    }
  }
  entity.clear();
}
