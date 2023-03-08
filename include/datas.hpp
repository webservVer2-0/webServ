#ifndef datas_hpp
#define datas_hpp

#include "config.hpp"
#include "webserv.hpp"

class ServerConfig;
class s_server_type;
class s_client_type;
class s_work_type;

typedef struct s_server t_server;

/**
 * @brief 이벤트 type을 정의하기 위한 enum
 *
 */
typedef enum s_event_type { SERVER, CLIENT, WORK } t_event;

/**
 * @brief client 이벤트의 진행상황을 확인하기 위한(kevent 분기를 고려하여) enum
 *
 */
typedef enum s_stage {
  DEF,
  REQ_READY,
  REQ_FIN,
  GET_READY,
  GET_FIN,
  POST_READY,
  POST_SAVED,
  POST_FIN,
  DELETE_READY,
  DELETE_FIN,
  ERR_FIN,  // error case로 page를 전달해야 할 때 체크해야할 enum
  RES_READY,
  RES_FIN,
  END
} t_stage;

/**
 * @brief client의 내부에서 처리되는 enum으로 에러 상태를 표시한다. 서버의
 * 영속성을 위해, 에러시 exit 처리가 아닌 다른 적절한 처리 후 client를 핸들링
 * 해야 한다.
 *
 */
typedef enum s_error {
  NO_ERROR = 0,
  OK = 200,
  MOV_RDIR = 301,
  BAD_REQ = 400,
  FORBID = 403,
  NOT_FOUND = 404,
  NOT_IMPLE = 501,
  OLD_HTTP = 505,
  SYS_ERR = 500
} t_error;

typedef enum s_chore { file, cgi } t_chore;

typedef struct s_http {
  typedef std::map<std::string, std::string> http_line;
  http_line init_line_;
  http_line header_;
  size_t entity_length_;
  char* entity_;
} t_http;

/**
 * @brief 다형성을 활용하여 제작한 클래스입니다. 해당 클래스는 type과 fd 를 갖고
 * 있으므로써, server, client, file, pipe 를 구분지으며, 작업에 따라 kevent
 * 구조체의 udata* 에 담음으로써, 데이터의 지속적인 감시 및 활용성을
 * 고려했습니다.
 *
 */
class s_base_type {
 private:
  t_event type_;
  int fd_;

  s_base_type(const s_base_type& target);
  s_base_type& operator=(const s_base_type& target);

 public:
  s_base_type(int fd);
  virtual ~s_base_type() {}

  void SetType(t_event val);
  t_event GetType() const;
  int GetFD();
};

/**
 * @brief 서버 구조체의 역할은 kevent 인 서버의 udata에 등록되어있는다. 그리하여
 * client가 등록되면 이와 동시에 client의 udata를 만들어, EV_SET 함과 동시에
 * 해당 데이터 세팅을 끝내는 역할을 합니다. 따라서 가지고 있어야 하는 것은, 각
 * 서버에 대한 컨피그 파일의 데이터입니다.
 *
 *
 */

class s_server_type : public s_base_type {
 private:
  t_server* self_config_;

  s_server_type(const s_server_type& target);
  s_server_type& operator=(const s_server_type& target);

 public:
  s_server_type(ServerConfig& config_list, int server_number, int server_fd);
  ~s_server_type();

  /**
   * @brief s_base_type 을 기반으로 client udata를 위한 포인터를 생성해낸다.
   *
   * @param client_fd 해당 내용을 위한 fd 값
   * @return s_base_type*
   */
  s_base_type* CreateClient(int client_fd);
};

/**
 * @brief client의 클래스로, 다형성 관리가 되도록 만들어져 있으며, 그 외에 http
 * 메시지, 에러 핸들링, 그 밖에 클라이언트의 이벤트 관리용 클래스이다. 서버가
 * client fd를 생성함과 동시에 연결되고, udata에 등록하여 이벤트의 추가 데이터로
 * 활영한다.
 *
 */
class s_client_type : public s_base_type {
 private:
  int cookie_id_;
  t_server* config_ptr_;
  t_http request_msg_;
  t_http response_msg_;

  s_base_type* parent_ptr_;
  s_base_type* data_ptr_;

  t_stage stage_;
  t_error status_code_;
  size_t msg_length;
  std::string err_custom_;  // custom msg 보관용
  int errno_;  // errno 발생시 해당 errno 를 넣어서 입력한다.

  s_client_type(const s_client_type& target, const t_server& master_config);
  s_client_type& operator=(const s_client_type& target);

 public:
  s_client_type(t_server* config, int client_fd, s_server_type* mother_ptr);
  ~s_client_type();

  /**
   * @brief 파생되는 파일을 열고 작업을 진행하기 위해 만들어진 객체. file, pipe
   * fd 를 위한 클래스
   *
   * @param path
   * @param file_fd
   * @param work_type
   * @return s_base_type*
   */
  s_base_type* CreateWork(std::string* path, int file_fd, s_chore work_type);

  int GetCookieId(void);
  t_http& GetRequest(void);
  t_http& GetResponse(void);
  void SetResponse(void);

  size_t& GetMessageLength(void);
  void SetMessageLength(size_t);

  const t_stage& GetStage(void);
  void SetStage(t_stage val);

  const t_error& GetErrorCode(void);
  void SetErrorCode(t_error val);

  const t_server& GetConfig(void);
  const s_server_type& GetParentServer(void);
  s_work_type* GetChildWork(void);

  bool GetCachePage(const std::string& uri, t_http& response);
  bool GetCacheError(t_error code, t_http& response);
};

/**
 * @brief 작업 클래스로, 파일을 열었거나, CGI를 요청했거나, 파일을 업로드 했을
 * 때 생성되는 클래스로, 그 이벤트를 위한 udata 용으로 활용한다.
 *
 */
class s_work_type : public s_base_type {
 private:
  const std::string uri_;
  s_base_type* client_ptr_;
  s_chore work_type_;

  s_work_type(const s_work_type& target);
  s_work_type& operator=(const s_work_type& target);

 public:
  s_work_type(std::string& path, int fd, s_chore work_type);
  ~s_work_type();
  void SetClientPtr(s_base_type* ptr);
  s_base_type* GetClientPtr(void);
  const std::string& GetUri(void);
  s_chore GetWorkType(void);
};

#endif
