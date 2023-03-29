#include "../../include/utils.hpp"
#include "../../include/webserv.hpp"

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

  std::set<std::string> d_set;
  std::set<std::string> f_set;

  std::string origin_uri = client->GetOriginURI();
  DIR* dir;
  struct dirent* ent;
  dir = opendir(directory_path.c_str());
  if (dir != NULL) {
    ent = readdir(dir);
    while (ent != NULL) {
      if (IsDirectory(ent)) {
        std::string location(client->GetLocationConfig().location_);
        std::string dot(".");
        std::string dot_dot("..");
        if ((strcmp(ent->d_name, dot.c_str()) == 0) ||
            ((origin_uri == location) &&
             (strcmp(ent->d_name, dot_dot.c_str()) == 0))) {
          ent = readdir(dir);
          continue;
        }
        d_set.insert(ent->d_name);
      } else if (IsFile(ent)) {
        f_set.insert(ent->d_name);
      }
      ent = readdir(dir);
    }
  }

  entity.append("	<h1>");
  entity.append(origin_uri);
  entity.append("</h1>\n");

  entity.append("<div class=\"grid-container\">");

  std::set<std::string>::iterator d_it = d_set.begin();
  for (; d_it != d_set.end(); d_it++) {
    entity.append("<div class=\"grid-item\">");
    entity.append(
        "<img src=\"/asset/folder.png\" "
        "alt=\"아이콘\" width=\"32\" "
        "height=\"32\">");
    entity.append("<span><a href=\"");
    if ((*d_it).size() <= 2) {
      if (*d_it == "..") {
        size_t pos = origin_uri.rfind('/');
        entity.append(origin_uri.substr(0, pos));
      }
      entity.append("\"style=\"margin-left:10px;\">");
      entity.append(*d_it);
    } else {
      std::string temp = origin_uri;
      if (temp.compare("/") == 0) {
        temp.append("localhost/");
        temp.append(*d_it);
      } else {
        temp.append("/");
        temp.append(*d_it);
      }
      entity.append(temp.c_str());
      entity.append("\"style=\"margin-left:10px;\">");
      entity.append(*d_it);
    }
    entity.append("</a></span>");
    entity.append("</div>");
  }
  std::set<std::string>::iterator f_it = f_set.begin();
  for (; f_it != f_set.end(); f_it++) {
    entity.append("<div class=\"grid-item\">");
    entity.append(
        "    <img "
        "src=\"/asset/file.png\" "
        "alt=\"아이콘\" width=\"32\" "
        "height=\"32\">");
    entity.append("<span><a href=\"");

    std::string temp;
    temp.append(origin_uri);
    temp.append("/");
    temp.append(*f_it);
    entity.append(temp.c_str());
    entity.append("\"style=\"margin-left:10px;\"download>");
    entity.append(*f_it);
    entity.append("</a></span>");
    entity.append("</div>");
  }
  entity.append("</div>");
  d_set.clear();
  f_set.clear();

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
