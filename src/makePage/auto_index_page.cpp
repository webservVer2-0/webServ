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
  // entity.append(directory_path);
  entity.append(client->GetOriginURI());
  entity.append("</h1>\n");

  entity.append("<div class=\"grid-container\">");
  DIR* dir;
  struct dirent* ent;
  dir = opendir(directory_path.c_str());
  if (dir != NULL) {
    ent = readdir(dir);
    while (ent != NULL) {
      if (IsDirectory(ent)) {
        // std::cout << "client->GetLocationConfig().location_ : " << client->GetLocationConfig().location_ << std::endl;
        std::string root(client->GetLocationConfig().location_);
        std::string dot(".");
        std::string dot2("..");
        if ((strcmp(ent->d_name, dot.c_str()) == 0) || ((client->GetOriginURI() == root) && (strcmp(ent->d_name, dot2.c_str()) == 0)))
        {
          ent = readdir(dir);
          continue;
        }
        // std::cout << "ent->d_name " << ent->d_name << std::endl;
        // std::cout << "client->GetOriginURI() : " << client->GetOriginURI() << std::endl;
        // std::cout << "directory_path : " << directory_path << std::endl;
        // std::cout << "client->GetConfig().main_config_.find(ROOT)->second : " << "./" + client->GetConfig().main_config_.find(ROOT)->second << std::endl;
        entity.append("<div class=\"grid-item\">");
        entity.append(
            "<img src=\"/asset/folder.png\" "
            "alt=\"아이콘\" width=\"32\" "
            "height=\"32\">");
        entity.append("<span><a href=\"");
        if (ent->d_namlen <= 2) {
          std::string name(ent->d_name);
          if (name.find("..") == 0) {
            // std::string temp = directory_path;
            std::string temp = client->GetOriginURI();
            size_t pos = temp.rfind('/');
            entity.append(temp.substr(0, pos));
          } else if (name.find(".") == 0) {
            // entity.append(directory_path);
            entity.append(client->GetOriginURI());
          }
          entity.append("\"style=\"margin-left:10px;\">");
          entity.append(ent->d_name);

        } else {
          // std::string temp = client->GetLocationConfig().location_;
          std::string temp = client->GetOriginURI();
          if (temp.compare("/") == 0) {
            temp.append("localhost/");
            temp.append(ent->d_name);
          } else {
            temp.append("/");
            temp.append(ent->d_name);
          }
          // temp.append("/");
          entity.append(temp.c_str());
          entity.append("\"style=\"margin-left:10px;\">");
          entity.append(ent->d_name);
        }
      } else if (IsFile(ent)) {
        entity.append("<div class=\"grid-item\">");
        entity.append(
            "    <img "
            "src=\"/asset/file.png\" "
            "alt=\"아이콘\" width=\"32\" "
            "height=\"32\">");
        entity.append("<span><a href=\"");

        // std::string temp = client->GetLocationConfig().location_;
        std::string temp;
        // temp.append("/");
        temp.append(client->GetOriginURI());
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
