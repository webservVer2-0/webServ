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
  e_type loc_type_[3];      //
  e_autoindex index_mode_;  //
} t_loc;

typedef std::map<std::string, t_loc*> location_list;

/**
 * @brief server 를 위한 핵심 데이터 구조체, 서버 컨피그의 핵심을 담당한다.
 */
typedef struct s_server {
  std::string port_;                                // port number
  config_map main_config_;                          // main config value map
  std::map<std::string, t_loc*> location_configs_;  // location structure
  e_autoindex index_mode_;                          // index mode 여부 확인
} t_server;

/**
 * @brief server class, config reading, validation checking, server config
 * getter 역할을 담당한다.
 */
class ServerConfig {
 public:
  typedef config_map::iterator conf_iterator;
  typedef std::string conf_value;

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

  //   t_server& GetServerByPort(int Port);
  //   conf_value& GetServerConfValueByPort(int Port, const char* key);
  //   conf_value& GetServerConfValueByPort(int Port, const std::string&
  //   key);

  //   t_loc& GetLocationByPort(int Port, const char* uri);
  //   conf_value& GetLocationConfValueByPort(int port, const char* uri,
  //  const char* key);
  //   conf_value& GetLocationConfValueByPort(int port, const std::string&
  //   uri,
  //  const std::string& key);
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

  // TODO : 핵심 정보를 가져올 수 있는 getter
  // TODO : 메인 로직에서 필요시 되는 getter
  // TODO : request, response handling 을 위한 각 설정과 uri 별 getter
  // index.html 문서 캐시 클래스 만들기
};

#endif
