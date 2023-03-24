#include "../../include/utils.hpp"
#include "../../include/webserv.hpp"
#include "dirent.h"

void MakeAutoindexPage(s_client_type* client, t_http& response,
                       std::string directory_path) {
  MakeHead(response);
  MakeAutoindexBody(client, response, directory_path);
  MakeFooter(response);
}

void MakeAutoindexBody(s_client_type* client, t_http& response,
                       std::string directory_path) {
  std::string entity;

  for (size_t i = 0; i < response.entity_length_; i += 2) {
    entity.push_back(response.entity_[i]);
    if (i + 1 < response.entity_length_) {
      entity.push_back(response.entity_[i + 1]);
    }
  }
  delete[] response.entity_;

  entity.append("	<h1>");
  entity.append(directory_path);
  entity.append("</h1>\n");

  entity.append("<div class=\"grid-container\">");
  DIR* dir;
  struct dirent* ent;
  dir = opendir(directory_path.c_str());
  if (dir != NULL) {
    ent = readdir(dir);
    while (ent != NULL) {
      entity.append("<div class=\"grid-item\">");
      if (IsDirectory(ent)) {
        entity.append(
            "<img src=\"/asset/folder.png\" "
            "alt=\"아이콘\" width=\"32\" "
            "height=\"32\">");
        entity.append("<span><a href=\"");
        if (ent->d_namlen < 2) {
          std::string name(ent->d_name);
          if (name.find(".") == 0) {
            entity.append(directory_path);
          } else if (name.find("..") == 0) {
            std::string temp = directory_path;
            size_t pos = temp.rfind('\\');
            entity.append(temp.substr(0, pos));
          }
          entity.append("\"style=\"margin-left:10px;\">");
          entity.append(ent->d_name);

        } else {
          std::string temp = client->GetLocationConfig().location_;
          if (temp.compare("/") == 0) {
            temp.append("localhost/");
            temp.append(ent->d_name);
          } else {
            temp.append("/");
            temp.append(ent->d_name);
          }
          entity.append(temp.c_str());
          entity.append("\"style=\"margin-left:10px;\">");
          entity.append(ent->d_name);
        }
      } else if (IsFile(ent)) {
        entity.append(
            "    <img "
            "src=\"/asset/file.png\" "
            "alt=\"아이콘\" width=\"32\" "
            "height=\"32\">");
        entity.append("<span><a href=\"");

        std::string temp = client->GetLocationConfig().location_;
        temp.append("/");
        temp.append(ent->d_name);
        entity.append(temp.c_str());
        entity.append("\"style=\"margin-left:10px;\"download>");
        entity.append(ent->d_name);
      }
      entity.append("</a></span>");
      entity.append("</div>");

      //   entity.append("<span><a href=\"");

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
