#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include "webserv.hpp"

#define SER "server"
#define LOC "location"

#define LISTEN "listen"         // s
#define BODY "body_size"        // s
#define MAXCON "max_connect"    // s
#define ROOT "root"             // s l
#define DEFFILE "default_file"  // s l
#define UPLOAD "upload_path"    // s
#define ACCLOG "access_log"     // s
#define ERRLOG "error_log"      // s
#define SERNAME "server_name"   // s
#define TIMEOUT "timeout"       // s (0은 안됨, 무조건 존재해야함)
#define AUTOINDEX "auto_index"  // s l(선택, 값은 정확해야한다)
#define METHOD "method"         // s l
#define ERR "error"             // s l(선택, 값은 정확해야한다)
#define CGIFILE "cgi"           //   l
#define LOCNAME "location"      //   l
#define REDIR "redirection"     //   l

enum e_type { T_NULL, T_ROOT, T_REDIR, T_CGI };
enum e_autoindex { autodef, on, off };

typedef std::map<std::string, std::string> config_map;

typedef struct s_loc {
  std::string location_;
  config_map main_config_;
  e_type loc_type_[3];
  e_autoindex index_mode_;
} t_loc;

typedef std::map<std::string, t_loc*> location_list;

typedef struct s_server {
  std::string port_;
  config_map main_config_;
  std::map<std::string, t_loc*> location_configs_;
  e_autoindex index_mode_;
} t_server;

typedef unsigned long pos_t;

class ServerConfig {
 public:
  typedef config_map::iterator conf_iterator;
  typedef std::string conf_value;

 private:
  std::vector<t_server*> server_list_;
  int64_t server_number_;

 private:
  void ParssingServer(const char* config_data);
  bool CheckKeyWord(const std::string& target, pos_t pos, const char* keyword);
  pos_t ParssingServerLine(std::string& config_string, pos_t init_pos);
  config_map* GetServerConfigByPort(int Port);
  config_map& GetLocationConfigByPort(int Port, const std::string& uri);
  void ValidCheckMain(void);
  bool ValidCheckServer(int server_number, conf_iterator& error_log);
  bool ValidCheckLocation(int server_number, std::string location_name,
                          conf_iterator& error_log);
  bool ValidConfigNumber(conf_iterator& target, char* standard,
                         conf_iterator& error_log);
  bool ValidConfigFilePath(conf_iterator& target, conf_iterator& error_log);
  bool ValidConfigFile(conf_iterator& target, conf_iterator& error_log);
  bool ValidConfigCGIFile(conf_iterator& target, conf_iterator& error_log,
                          t_loc& location);
  bool ValidConfigStr(conf_iterator& target, conf_iterator& error_log);
  bool ValidConfigHTML(conf_iterator& target, conf_iterator& error_log);
  bool ValidConfigAutoindex(conf_iterator& target, conf_iterator& error_log,
                            int server_number);
  bool ValidConfigError(conf_iterator& target, conf_iterator& error_log);
  bool ValidConfigAutoindexLocation(conf_iterator& target,
                                    conf_iterator& error_log, t_loc& location);

 public:
  ServerConfig(const char* confpath);
  //   ~ServerConfig();
  ssize_t PrintServerConfig();

  t_server* GetServerByPort(int Port);
  conf_value* GetServerConfValueByPort(int Port, const char* key);
  conf_value* GetServerConfValueByPort(int Port, const std::string& key);

  t_loc* GetLocationByPort(int Port, const char* uri);
  conf_value* GetLocationConfValueByPort(int port, const char* uri,
                                         const char* key);
  conf_value* GetLocationConfValueByPort(int port, const std::string& uri,
                                         const std::string& key);

  // TODO : port 기준으로 서버 구조체 포인터 게터
  // TODO : port, uri 넣어주면 로케이션 구조체 포인터 반환 게터
  // TODO : 포트 + 키 를 넣으면 서버의 키 value 를 반환하는 게터
  // TODO  : port, uri, key 넣어주면 로케이션의 키 value 를 반환하는 게터(없으면
  // 서버 디폴트값을 찾아줌)
};

#endif
