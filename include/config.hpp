#ifndef CONFIG_HPP_
#define CONFIG_HPP_

#include "cache.hpp"
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
#define INC "include"

typedef std::string extension;
typedef std::string mime_type;

typedef std::map<extension, mime_type> t_mime;
typedef std::pair<extension, mime_type> pair_mime;
typedef t_mime::iterator mime_iterator;

typedef std::string path;
typedef char* cache_entity;

typedef std::map<path, cache_entity> t_cache;
typedef std::pair<path, cache_entity> pair_cache;
typedef t_cache::iterator cache_iterator;

/**
 * @brief 해당 로케이션이 어떤 타입의 로케이션인지를 구분짓는 역할을 함.
 */
enum e_type { T_NULL, T_ROOT, T_REDIR, T_CGI };

/**
 * @brief auto index 모드를 구현하기 위한 플래그.
 */
enum e_autoindex { autodef, on, off };

/**
 * @brief key, value 구분을 위한 기초 자료 구조
 */
typedef std::map<std::string, std::string> config_map;

/**
 * @brief 위치 정보용 데이터 alias
 */
typedef unsigned long pos_t;

/**
 * @brief location 별 데이터구조를 담당한다.
 */
typedef struct s_loc {
  std::string location_;    // location name
  config_map main_config_;  // location config value map
  e_type loc_type_[3];  // location 타입들이 순서대로 들어간다. ROOT, REDIR, CGI
  e_autoindex index_mode_;  //
} t_loc;

typedef std::map<std::string, t_loc*> location_list;

/**
 * @brief server 를 위한 핵심 데이터 구조체, 서버 컨피그의 핵심을 담당한다.
 */
typedef struct s_server {
  std::string port_;                // port number
  config_map main_config_;          // main config value map
  location_list location_configs_;  // location structure
  int server_fd_;                   // server fd
  e_autoindex index_mode_;          // index mode 여부 확인
                                    // TODO: cache setting 넣기
  t_cache static_pages_;            // TODO: static cache 페이지들 저장용
  t_cache error_pages_;             // TODO: error cache 페이지들 저장용
  t_mime mime_;                     // TODO: mime 작성하기

} t_server;

/**
 * @brief server class, config reading, validation checking, server config
 * getter 역할을 담당한다.
 */
class ServerConfig {
 public:
  typedef std::vector<t_server*>::iterator serv_iterator;
  typedef config_map::iterator conf_iterator;

 private:
  std::vector<t_server*> server_list_;
  int* server_socket_;
  struct sockaddr_in* server_addr_;
  int64_t server_number_;
  int kq_;

 private:
  void ParssingServer(const char* config_data);
  bool CheckKeyWord(const std::string& target, pos_t pos, const char* keyword);
  pos_t ParssingServerLine(std::string& config_string, pos_t init_pos);
  config_map* GetServerConfigByPort(int Port);
  config_map& GetLocationConfigByPort(int Port, const std::string& uri);
  void ValidCheckMain(void);
  bool ValidCheckServer(int server_number);
  bool ValidCheckLocation(int server_number, std::string location_name,
                          conf_iterator& error_log);
  bool ValidConfigNumber(conf_iterator& target, const char* standard);
  bool ValidConfigFilePath(conf_iterator& target);
  bool ValidConfigFile(conf_iterator& target);
  bool ValidConfigCGIFile(conf_iterator& target, conf_iterator& error_log,
                          t_loc& location);
  bool ValidConfigStr(conf_iterator& target);
  bool ValidConfigHTML(conf_iterator& target);
  bool ValidConfigAutoindex(conf_iterator& target, int server_number);
  bool ValidConfigError(conf_iterator& target);
  bool ValidConfigAutoindexLocation(conf_iterator& target,
                                    conf_iterator& error_log, t_loc& location);
  void ServerAddressInit(void);
  void ServerSocketInit(void);
  void ServerEventInit(void);

 public:
  std::vector<struct kevent> change_list_;
  struct kevent* event_list_;
  int max_connection;

  ServerConfig(const char* confpath);
  ~ServerConfig(void);
  ssize_t PrintServerConfig(void);
  /**
   * @brief 서버 별 정보를 호출한다.
   *
   * @param num
   */
  void PrintTServer(int num);

  int* GetServerSocket(void);
  int GetServerNumber(void);
  int GetServerPort(int number);
  struct sockaddr_in* GetServerAddress(void);
  void SetServerKque(int que);
  int GetServerKque(void);

  const t_server& GetServerList(int number);
  /**
   * @brief Get the Server Config By Number object
   *
   * @param number
   * @return const t_server* , error = NULL
   */
  const t_server* GetServerConfigByNumber(int number);
  /**
   * @brief Get the Server Config By Port object
   *
   * @param port
   * @return const t_server* , error = NULL
   */
  const t_server* GetServerConfigByPort(const std::string& port);

  t_mime& GetServerMimeByNumber(int number);
};

#endif
