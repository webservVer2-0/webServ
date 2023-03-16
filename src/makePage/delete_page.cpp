#include "../../include/utils.hpp"
#include "../../include/webserv.hpp"
#include "dirent.h"

void MakeDeletePage(s_client_type* client, t_http& response,
                    std::string directory_path) {
  MakeHead(response);
  MakeDeleteBody(client, directory_path, response);
  MakeFooter(response);
}

void MakeDeleteBody(s_client_type* client, std::string directory_path,
                    t_http& response) {
  std::string entity(response.entity_);
  delete[] response.entity_;

  entity.append("	<h1>");
  entity.append("DELETE_PAGE");
  entity.append("</h1>\n");

  entity.append("<div class=\"grid-container\">");

  DIR* dir;
  struct dirent* ent;
  dir = opendir(directory_path.c_str());
  if (dir != NULL) {
    ent = readdir(dir);
    while (ent != NULL) {
      if (IsDirectory(ent)) {
        continue;
      } else if (IsFile(ent)) {
        entity.append("  <div class=\"grid-item\">");
        entity.append(
            "<img src = \"./asset/folder.png\" alt = \"아이콘\" width=\"32\" "
            "height=\"32\">");
        entity.append("    <span style=\"margin-left:10px;\">");

        std::string temp =
            MakeNameWithoutID(client->GetCookieId(), std::string(ent->d_name));
        entity.append(temp);
        entity.append("</span>");
        entity.append(
            "<form method=\"DELETE\" action=\"/delete\" "
            "accept-charset=\"ASCII\"><input type =\"hidden\" "
            "name=\"delete_target\" value=\"");
        entity.append(ent->d_name);
        entity.append("\">");
        entity.append(
            "<button type=\"submit\"><img src=\"./asset/bin.png\" "
            "style=\"margin-left: 10px; margin-top: 5px;\" "
            "width=\"16px;\"></button></form>");
      }
      ent = readdir(dir);
    }
  }
  entity.append("</div>");

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
