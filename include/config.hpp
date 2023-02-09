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
#define TIMEOUT "timeout"       // s
#define AUTOINDEX "auto_index"  // s l
#define METHOD "method"         // s l
#define ERR "error"             // s l
#define CGIFILE "cgi"           //   l
#define LOCNAME "location"      //   l
#define REDIR "redirection"     //   l

enum e_server {
  Listen,
  BodySize,
  MaxConnect,
  Root,
  DefaultFile,
  UploadPath,
  AccessLog,
  ErrorLog,
  ServerName,
  Timeout,
  AutoIndex,
  Method,
  Error
};

enum e_location {
  locationname,
  cgi,
  redirection,
  root,
  defaultfile,
  autoindex,
  method,
  error
};

enum e_type { T_ROOT, T_LOCATION, T_CGI };
enum e_autoindex { on, off };

typedef struct s_loc_conf {
  std::map<std::string, std::string> configs_;
  e_type loc_type_;
  e_autoindex mode_;
} t_loc_conf;

typedef std::map<std::string, t_loc_conf*> loc_list;

typedef struct s_ser_conf {
  std::map<std::string, std::string> main_config_;
  loc_list location_;
} t_ser_conf;

typedef unsigned long pos_t;

class ServerConfig {
 public:
  typedef std::vector<std::string> string_list;
  typedef std::map<int, string_list> keywords_type;

 private:
  std::vector<t_ser_conf*> server_list_;
  int64_t server_number_;

 private:
  void ParssingServer(const char* config_data);
  bool CheckKeyWord(const std::string& target, pos_t pos, const char* keyword);
  pos_t ParssingServerLine(keywords_type server_list,
                           keywords_type location_list,
                           std::string& config_string, pos_t init_pos);
  void InitKeywords(keywords_type& list, int code);

 public:
  ServerConfig(const char* confpath);
  //   ~ServerConfig();
  //   ssize_t PrintServerConfig();
  //   t_ser_conf* GetServer(int64_t server_number);
  //   t_ser_conf* GetServer(const char* server_name);
  //   loc_list::iterator GetServerLocation(int64_t server_number);
  //   loc_list::iterator GetServerLocation(const char* server_name);
};

#endif
