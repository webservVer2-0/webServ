#include <iostream>
#include <sstream>
#include <string>

#include "../../include/webserv.hpp"

#define CRLF "\r\n"
#define DOUBLE_CRLF "\r\n\r\n"
#define CRLF_LEN 2
#define DOUBLE_CRLF_LEN 4

typedef enum t_method {
  GET,
  POST,
  DELETE,
} e_method;

typedef struct s_elem {
  std::string _init_line;
  std::string _header_line;
  char* _entity;
  e_method _method;
  bool _error;
  size_t _content_length;
  size_t _header_end;
} t_elem;

inline std::string int_to_string(int num) {
  std::stringstream ss;
  ss << num;
  std::string s = ss.str();
  return s;
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
  size_t first_space = e->_header_line.find(" ");
  if (first_space == std::string::npos) {
    return (BAD_REQ);
  }
  try {
    http->init_line_["METHOD"] = e->_header_line.substr(0, first_space);
  } catch (std::exception) {
    return (SYS_ERR);
  }

  size_t second_space = e->_header_line.find(" ", first_space + 1);
  if (second_space == std::string::npos) {
    return (BAD_REQ);
  }
  try {
    http->init_line_["URI"] =
        e->_header_line.substr(first_space + 1, second_space - first_space - 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  std::string http_ver;
  try {
    http_ver = e->_header_line.substr(
        second_space + 1, e->_header_line.size() - second_space + 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  if (http_ver == "HTTP/0.9" || http_ver == "HTTP/1.0") {
    return (OLD_HTTP);
  }
  if (http_ver != "HTTP/1.1") {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

void recursive_fill_header(t_http* http, t_elem* elem, std::string line,
                           size_t s, size_t e) {
  int content_len_flag = 0;
  int end_flag = 0;
  std::string no_escape_line;
  size_t colon_pos;

  e = line.find(CRLF, s);
  if (e == std::string::npos) {
    e = line.size();
    end_flag = 1;
  }
  try {
    no_escape_line = line.substr(s, e - s);
    /*
      post면 체크
    */
    if (elem->_method == POST) {
      if (no_escape_line.find("Content-Length: ") != std::string::npos) {
        content_len_flag = 1;
      }
    }
    colon_pos = no_escape_line.find(":");
    if (colon_pos != std::string::npos) {
      std::string key = no_escape_line.substr(0, colon_pos);
      std::string value = no_escape_line.substr(colon_pos + strlen(": "));
      http->header_[key] = value;
      if (content_len_flag) {
        elem->_content_length = atoi(value.c_str());
      }
    } else {
      elem->_error = true;
      return;
    }
    if (end_flag) {
      return;
    }
  } catch (std::exception) {
    elem->_error = true;
    return;
  }
  recursive_fill_header(http, elem, line, s + e + CRLF_LEN, e);
}

void parser(char* msg, t_http* http) {
  if (!msg) {
    return;  // error
  }
  t_elem e;
  memset(&e, 0, sizeof(t_elem));
  std::string line(msg);
  size_t init_end = line.find(CRLF);
  if (init_end == std::string::npos) {
    return;  // error
  }
  try {
    e._init_line = line.substr(0, init_end - CRLF_LEN);
  } catch (std::out_of_range) {
    return;  // error;
  }
  if (init_line_parser(http, &e)) {
    return;  // error;
  } else {
    /* t_http에 넣기 */
  }
  e._header_end = line.find(DOUBLE_CRLF);
  if (e._header_end == std::string::npos) {
    return;  // error
  }
  try {
    // e.e->_header_line = line.substr(init_end + CRLF_LEN, ._header_end);
  } catch (std::out_of_range) {
    return;  // error
  }
  recursive_fill_header(http, &e, e._header_line, 0, 0);
  if (e._error) {
    return;  // error;
  }
  /*
    entity
  */
  /*

   jinypark님 파트

  */
  return;
}

t_error fun_test(t_http* http) {
  std::string _header_line = "POST /index.html HTTP/1.1 ";

  size_t first_space = _header_line.find(" ");
  if (first_space == std::string::npos) {
    return (BAD_REQ);
  }
  try {
    http->init_line_["METHOD"] = _header_line.substr(0, first_space);
  } catch (std::exception) {
    return (SYS_ERR);
  }

  size_t second_space = _header_line.find(" ", first_space + 1);
  if (second_space == std::string::npos) {
    return (BAD_REQ);
  }
  try {
    http->init_line_["URI"] =
        _header_line.substr(first_space + 1, second_space - first_space - 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  std::string http_ver;
  try {
    http_ver = _header_line.substr(second_space + 1,
                                   _header_line.size() - second_space + 1);
  } catch (std::exception) {
    return (SYS_ERR);
  }
  if (http_ver == "HTTP/0.9" || http_ver == "HTTP/1.0") {
    return (OLD_HTTP);
  }
  if (http_ver != "HTTP/1.1") {
    return (BAD_REQ);
  }
  return (NO_ERROR);
}

int main(void) {
  char* rq =
      "POST /index.html HTTP/1.1\r\n"
      "Host : www.example.com\r\nContent-Type: "
      "application/x-www-form-urlencoded\r\nContent-Length: "
      "26\r\n\r\nname=John+Doe&age=25&sex=M";
  t_elem e;
  t_http h;
  t_error er;
  if ((er = fun_test(&h))) {
    std::cout << "error : " << (int)er;
  }
  parser(rq, &h);
}