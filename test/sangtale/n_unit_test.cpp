#include <iostream>
#include <sstream>
#include <string>

#include "../../include/webserv.hpp"

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF_LEN 2
#define DOUBLE_CRLF_LEN 4

typedef enum t_method { GET, POST, DELETE } e_method;

typedef struct s_elem {
  std::string _init_line;
  std::string _header_line;
  e_method _method;
  size_t _content_length;
  size_t _header_end;
} t_elem;

template <typename T>
std::string to_string(const T& value) {
  std::ostringstream oss;
  oss << value;
  return oss.str();
}

int method_parser(t_elem* e) {
  const std::string requisite[3] = {"GET", "POST", "DELETE"};

  for (int i = 0; i < 3; ++i) {
    if (e->_init_line.find(requisite[i]) != std::string::npos) {
      e->_method = static_cast<e_method>(i);
      return (0);
    }
  }
  return (1);
}

t_error init_line_parser(t_http* http, t_elem* e) {
  if (method_parser(e)) {
    return (NOT_IMPLE);
  }
  size_t first_space = e->_init_line.find(" ");
  if (first_space == std::string::npos) {
    return (BAD_REQ);
  }
  try {
    http->init_line_["METHOD"] = e->_init_line.substr(0, first_space);
  } catch (std::exception) {
    return (SYS_ERR);
  }

  size_t second_space = e->_init_line.find(" ", first_space + 1);
  if (second_space == std::string::npos) {
    return (BAD_REQ);
  }
  try {
    http->init_line_["URI"] =
        e->_init_line.substr(first_space + 1, second_space - first_space - 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  std::string http_ver;
  try {
    // std::cout << "e->init_line : " << e->_init_line << "\n";
    http_ver = e->_init_line.substr(second_space + 1,
                                    e->_init_line.size() - second_space + 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  // std::cout << "HTTP_VER: " << http_ver << "\n";
  if (http_ver == "HTTP/0.9" || http_ver == "HTTP/1.0") {
    return (OLD_HTTP);
  }
  if (http_ver != "HTTP/1.1") {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

t_error malloc_entity(t_http* http, t_elem* e, unsigned char* client_msg) {
  ssize_t entity_len;
  try {
    entity_len = std::stol(http->header_["Content-Length"]);
  } catch (std::invalid_argument const& ex) {
    return (BAD_REQ);
  }
  if (entity_len < 0) {
    return (BAD_REQ);
  }
  http->entity_length_ = entity_len;
  try {
    size_t entity_start = e->_header_end + 4;
    http->entity_ = new unsigned char[entity_len + 1];
    memcpy(http->entity_, client_msg + entity_start, entity_len);
    http->entity_[entity_len + 1] = '\0';
    return (NO_ERROR);
  } catch (std::bad_alloc& e) {
    return (SYS_ERR);
  }
}

/*
      "Host: www.example.com\r\n
      Content-Type: application/x-www-form-urlencoded\r\n
      Content-Length: 26\r\n\r\n"
*/
t_error fill_header(t_http* http, t_elem* e) {
  size_t pos = 0;
  size_t end_pos;
  std::string key;
  std::string value;

  try {
    while ((end_pos = e->_header_line.find(CRLF, pos)) != std::string::npos) {
      std::string line = e->_header_line.substr(pos, end_pos - pos);

      size_t colon_pos = line.find(": ");
      if (colon_pos != std::string::npos) {
        key = line.substr(0, colon_pos);
        value = line.substr(colon_pos + CRLF_LEN);
        http->header_[key] = value;
      } else if (pos == end_pos) {
        break;
      } else {
        return (BAD_REQ);
      }
      pos = end_pos + CRLF_LEN;
    }
  } catch (std::exception& e) {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

t_error elem_initializer(t_elem* e, std::string line) {
  memset(e, 0, sizeof(t_elem));

  if (line.empty()) {
    return (BAD_REQ);
  }
  size_t init_end = line.find(CRLF);
  if (init_end == std::string::npos) {
    return (BAD_REQ);
  }
  e->_header_end = line.find(DOUBLE_CRLF);
  if (e->_header_end == std::string::npos) {
    return (BAD_REQ);
  }
  e->_header_line =
      line.substr(init_end + CRLF_LEN, e->_header_end - init_end + CRLF_LEN);
  e->_init_line = line.substr(0, init_end);
  return (NO_ERROR);
}

void parser(unsigned char* msg, t_http* http) {
  if (!msg) {
    return;  // error
  }
  t_elem e;
  std::string line((char*)msg);
  try {
    if (elem_initializer(&e, line)) {
      return;
    }
    if (init_line_parser(http, &e)) {
      return;  // error;
    }
    if (fill_header(http, &e)) {
      return;
    }
    if (e._method == POST) {
      if (malloc_entity(http, &e, msg)) {
        return;  // error
      }
    }
  } catch (std::exception& e) {
    return;
  }
  return;
}

void show_map(std::map<std::string, std::string> map) {
  std::map<std::string, std::string>::iterator it;
  for (it = map.begin(); it != map.end(); it++) {
    std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
  }
}

void print(t_http* h) {
  std::cout << "init_line [METHOD] : " << h->init_line_["METHOD"] << "\n";
  std::cout << "init_line [URI] : " << h->init_line_["URI"] << "\n";
  std::cout << "entity_ : " << h->entity_ << "\n";
  std::cout << "entity_length : " << h->entity_length_ << "\n";
  std::cout << "strlen() : " << strlen((char*)h->entity_) << "\n";
  std::cout << "[Header]\n";
  show_map(h->header_);
  std::cout << "[end]\n";
  delete[] h->entity_;
}

int main(void) {
  unsigned char* rq = (unsigned char*)
      "POST /index.html HTTP/1.1\r\n"
      "Host: www.example.com\r\nContent-Type: "
      "application/x-www-form-urlencoded\r\nContent-Length: "
      "30\r\n\r\nname=John+Doe&age=25&sex=M";

  t_elem e;
  t_http h;
  t_error er;
  parser(rq, &h);
  print(&h);
  return 0;
}
