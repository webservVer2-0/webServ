#include "../../include/utils.hpp"
#include "../../include/webserv.hpp"
#include "dirent.h"

void MakeAutoindexPage(t_http& response, std::string directory_path) {
  MakeHead(response);
  MakeAutoindexBody(response, directory_path);
  MakeFooter(response);
}

void MakeAutoindexBody(t_http& response, std::string directory_path) {
  std::string entity(response.entity_);
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
            "<img src=\"http://localhost:80/storage/static/asset/folder.png\" "
            "alt=\"아이콘\" width=\"32\" "
            "height=\"32\">");
      } else if (IsFile(ent)) {
        entity.append(
            "    <img "
            "src=\"http://localhost:80/storage/static/asset/folder.png\" "
            "alt=\"아이콘\" width=\"32\" "
            "height=\"32\">");
      }
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
        std::string temp = directory_path;
        temp.append(ent->d_name);
        entity.append(temp.c_str());
        entity.append("\"style=\"margin-left:10px;\">");
        entity.append(ent->d_name);
      }
      entity.append("</a></span>");
      entity.append("</div>");

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