#include "../../include/datas.hpp"
#include "../../include/utils.hpp"
#include "../../include/webserv.hpp"

void MakeHead(t_http& response) {
  std::string entity;
  entity.append("<!DOCTYPE html>\n");
  entity.append("<html lang=\"en\">\n");
  entity.append("<head>\n");
  entity.append("	<meta charset=\"UTF-8\">\n");
  entity.append(
      "	<meta http-equiv=\"X-UA-Compatible\"content=\"IE=edge "
      "\">\n");
  entity.append(
      "	<meta name=\"viewport\" content=\"width=device-width, "
      "initial-scale=1.0\">\n");
  entity.append(
      "	<link rel=\"stylesheet\" type=\"text/css\" "
      "href=\"/asset/style.css\">\n");
  entity.append("	<title>류진스의 길었던 하류</title>\n");
  entity.append(
      "    <link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" "
      "href=\"/asset/favicon_io/favicon-32x32.png\">\n");
  entity.append(
      "    <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" "
      "href=\"/asset/favicon_io/favicon-16x16.png\">\n");
  entity.append("</head>\n");
  entity.append("<body>\n");
  entity.append(
      "	<div id=\"background\"><img src=\"/asset/wallpaper.jpeg\" "
      "id=\"back_img\"></img></div>\n");
  entity.append("	<div class=\"box\" >\n");
  entity.append("	<a href=\"/\">\n");
  entity.append("		<h1 id=\"banner\">류진스의 길었던 하류</h1>\n");
  entity.append("		<h2 id=\"banner_mid\" >Webserv Project</h2>\n");
  entity.append(
      "		<span id=\"banner_under\">우리는 생각한다. 고로 "
      "웹서브를해낸다</span>\n");
  entity.append("		</a>\n");
  entity.append("	</div>\n");

  response.entity_ = new char[entity.size()];
  response.entity_length_ = entity.size();
  for (size_t i = 0; i < response.entity_length_; i += 2) {
    response.entity_[i] = entity[i];
    if (i + 1 < response.entity_length_)
      response.entity_[i + 1] = entity[i + 1];
  }
  entity.clear();
}

void MakeFooter(t_http& response) {
  std::string entity;

  for (size_t i = 0; i < response.entity_length_; i += 2) {
    entity.push_back(response.entity_[i]);
    if (i + 1 < response.entity_length_) {
      entity.push_back(response.entity_[i + 1]);
    }
  }
  delete[] response.entity_;

  entity.append(
      "<a href=\"/\"><img src=\"/asset/home.png\" "
      "alt=\"홈버튼\"id=\"home\"></img></a>\n");
  entity.append("</body>\n</html>");

  response.entity_ = new char[entity.size() + 1];
  response.entity_length_ = entity.size();

  for (size_t i = 0; i < response.entity_length_; i += 2) {
    response.entity_[i] = entity[i];
    if (i + 1 < response.entity_length_)
      response.entity_[i + 1] = entity[i + 1];
  }

  response.entity_[response.entity_length_] = '\0';
  entity.clear();
}

std::string MakeNameWithoutID(std::string cookie_id, std::string file_name) {
  std::string ret;

  if (file_name.find(cookie_id) == 0) {
    int size = file_name.size();
    ret = file_name.substr(cookie_id.size() + 1, size - cookie_id.size() - 1);
  } else {
    ret = file_name;
  }
  return ret;
}

bool IsFile(struct dirent* target) {
  if (target->d_type == DT_REG) {
    return true;
  }
  return false;
}

bool IsDirectory(struct dirent* target) {
  if (target->d_type == DT_DIR) {
    return true;
  }
  return false;
}
