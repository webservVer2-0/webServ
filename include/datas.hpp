#ifndef datas_hpp
#define datas_hpp

#include "config.hpp"
#include "webserv.hpp"

class ServerConfig;
// class s_base_type;
// class s_server_type;
// class s_client_type;
// class s_work_type;
// class s_logger_type;

typedef struct s_server t_server;
typedef struct s_loc t_loc;

/**
 * @brief 이벤트 type을 정의하기 위한 enum
 *
 */
typedef enum s_event_type { SERVER, CLIENT, WORK, LOGGER } t_event;

/**
 * @brief client 이벤트의 진행상황을 확인하기 위한(kevent 분기를 고려하여) enum
 *
 */
typedef enum s_stage {
  DEF,
  REQ_READY,
  GET_READY,
  GET_START,
  GET_CHUNK,
  GET_FIN,
  POST_READY,
  POST_START,
  POST_SAVED,
  POST_FIN,
  DELETE_READY,
  DELETE_START,
  DELETE_FIN,
  ERR_FIN,  // error case로 page를 전달해야 할 때 체크해야할 enum
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
  MOV_PERMAN = 308,
  BAD_REQ = 400,
  FORBID = 403,
  NOT_FOUND = 404,
  NOT_IMPLE = 501,
  OLD_HTTP = 505,
  SYS_ERR = 999
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

class s_work_type : public s_base_type {
 private:
  const std::string uri_;
  s_base_type* client_ptr_;
  s_chore work_type_;
  t_http& response_msg_;

  s_work_type(const s_work_type& target);
  s_work_type& operator=(const s_work_type& target);

 public:
  s_work_type(std::string& path, int fd, s_chore work_type,
              t_http& response_msg);
  ~s_work_type();
  void SetClientPtr(s_base_type* ptr);
  s_base_type* GetClientPtr(void);
  const std::string& GetUri(void);

  s_chore GetWorkType(void);
  t_http& GetResponseMsg(void);
  /**
   * @brief File fd에서 해당 작업이 끝나고 나면, client 처리를 위해 사용하는
   * 함수입니다. 설정에 필요한 fliter~udata까지 집어 넣으시면 됩니다. 신규
   * 등록이 아니니 udata는 NULL 로 넣어 됩니다.
   *
   * @param filter
   * @param flags
   * @param fflags
   * @param data
   * @param udata
   */
  void ChangeClientEvent(int16_t filter, uint16_t flags, uint16_t fflags,
                         intptr_t data, void* udata);

  t_stage GetClientStage(void);
  void SetClientStage(t_stage val);
};

class s_logger_type : public s_base_type {
 private:
  int logging_fd_;
  int error_fd_;
  std::vector<std::string> logs_;

 public:
  s_logger_type();
  ~s_logger_type();
  //   s_logger_type& operator=(const s_logger_type& target);
  void SetFDs(int log_fd, int err_fd);
  int GetLoggingFd(void);
  int GetErrorFd(void);
  void GetData(std::string);
  void PushData(void);
  void PrintLogger(void);
};

class s_server_type : public s_base_type {
 private:
  t_server* self_config_;
  s_logger_type logger_;

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
  s_logger_type& GetLogger(void) { return this->logger_; }
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
  std::string cookie_id_;  // 접속한 클라이언트에 대해 부여하는 고유한 넘버
  std::string ip_;            // TODO: logger
  std::time_t time_data_[2];  // 0 - access time, 1 - stage change time
                              // TODO: logger

  t_server* config_ptr_;  // 클라이언트에서 사용해야 하는 서버 설정
  t_loc* loc_config_ptr_;  // 클라이언트의 uri가 소속된 로케이션 설정

  std::string origin_uri_;  // TODO: logger
  t_http request_msg_;      // request 메시지
  t_http response_msg_;     // response 메시지
  size_t sent_size_;  // chunked encoding 상황에서 얼마나 전달했는지를 체크함

  s_base_type* parent_ptr_;  // server 클래스 포인터
  s_base_type* data_ptr_;  // work type으로 작업을 하는 영역의 클래스 포인터

  t_stage stage_;        // 현재의 작업 단계를 확인용 //TODO: logger
  t_error status_code_;  // HTTP status를 확인하는 용 //TODO: logger
  std::string err_custom_;  // custom msg 보관용 //TODO: logger
  int errno_;  // errno 발생시 해당 errno 를 넣어서 입력한다. //TODO: logger

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

  std::string GetCookieId(void);
  void SetCookieId(std::string prev_id);
  t_http& GetRequest(void);
  t_http& GetResponse(void);

  const t_stage& GetStage(void);
  void SetStage(t_stage val);

  const t_error& GetErrorCode(void);
  void SetErrorCode(t_error val);

  const t_server& GetConfig(void);
  const s_server_type& GetParentServer(void);
  const t_loc& GetLocationConfig(void);
  void SetConfigPtr(t_loc* ptr);
  s_work_type* GetChildWork(void);

  bool GetCachePage(const std::string& uri, t_http& response);
  bool GetCacheError(t_error code, t_http& response);

  bool GetChunked(void);
  size_t GetChunkSize(void);
  /**
   * @brief 전달한 사이즈만큼과 실제 전체 entity_length_를 비교하여 값을
   * 다보낸 경우 true, 아닌 경우 false를 보낸다.
   *
   * @param set_size
   * @return true
   * @return false
   */
  bool IncreaseChunked(size_t set_size);

  void SetOriginURI(std::string path);
  const std::string& GetOriginURI(void);

  void SetWorkFinishTime(void);
  void SetIP(const char* IP);
  const std::string& GetIP(void);

  std::time_t* GetTimeData(void);

  void SendLogs(void);

  void PrintClientStatus(void);
};

/**
 * @brief 작업 클래스로, 파일을 열었거나, CGI를 요청했거나, 파일을 업로드 했을
 * 때 생성되는 클래스로, 그 이벤트를 위한 udata 용으로 활용한다.
 *
 */

#endif
