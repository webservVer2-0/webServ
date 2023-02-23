#ifndef datas_hpp
#define datas_hpp

#include "webserv.hpp"
/**
 * @brief 이벤트 type을 정의하기 위한 enum
 *
 */
typedef enum s_s_type { SERVER, CLIENT, WORK } t_s_type;

/**
 * @brief client 이벤트의 진행상황을 확인하기 위한(kevent 분기를 고려하여) enum
 *
 */
typedef enum s_stage {
  REQ_READY,
  REQ_FIN,
  GET_READY,
  GET_FIN,
  POST_READY,
  POST_SAVED,
  POST_FIN,
  DELETE_READY,
  DELETE_FIN,
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
  OK = 200,
  BAD_REQ = 400,
  FORBID = 403,
  NOT_FOUND = 404,
  NOT_IMPLE = 501,
  OLD_HTTP = 505,
  SYS_ERR = 999
} t_error;

typedef struct s_html {
  std::map<std::string, std::string> init_line_;
  std::map<std::string, std::string> header_;
  char* entity;
} t_html;

/**
 * @brief 다형성을 활용하여 제작한 클래스입니다. 해당 클래스는 type과 fd 를 갖고
 * 있으므로써, server, client, file, pipe 를 구분지으며, 작업에 따라 kevent
 * 구조체의 udata* 에 담음으로써, 데이터의 지속적인 감시 및 활용성을
 * 고려했습니다.
 *
 */
class s_base {
 private:
  t_s_type type_;
  int fd_;

  s_base(const s_base& target);
  s_base& operator=(const s_base& target);

 public:
  s_base() { type_ = SERVER; };
  virtual ~s_base();

  void SetType(t_s_type val) { type_ = val; };
  void SetFD(int val);

  t_s_type GetType() const { return this->type_; };
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
class s_server : public s_base {
 private:
  // TODO: data structure setting
  t_server* self_config_;

  s_server(const s_server& target);
  s_server& operator=(const s_server& target);

 public:
  s_server();
  ~s_server();
  s_base* CreateClient(int client_fd);
};

/**
 * @brief client의 클래스로, 다형성 관리가 되도록 만들어져 있으며, 그 외에 http
 * 메시지, 에러 핸들링, 그 밖에 클라이언트의 이벤트 관리용 클래스이다. 서버가
 * client fd를 생성함과 동시에 연결되고, udata에 등록하여 이벤트의 추가 데이터로
 * 활영한다.
 *
 */
class s_client : public s_base {
 private:
  // TODO: data structure setting

  t_server* config_ptr_;
  t_html request_msg_;
  t_html response_msg_;

  s_base* parent_ptr_;
  s_base* data_ptr_;

  t_stage stage_;
  t_error http_status_code_;

  s_client(const s_client& target, const t_server& master_config);
  s_client& operator=(const s_client& target);

 public:
  s_client();
  ~s_client();
  s_base* CreateWork(int file_fd);
};

/**
 * @brief 작업 클래스로, 파일을 열었거나, CGI를 요청했거나, 파일을 업로드 했을
 * 때 생성되는 클래스로, 그 이벤트를 위한 udata 용으로 활용한다.
 *
 */
class s_work : public s_base {
 private:
  // TODO: data structure setting
  const std::string uri_;
  s_base* client_ptr_;

  s_work(const s_work& target);
  s_work& operator=(const s_work& target);

 public:
  s_work(std::string& path);
  ~s_work();
  void SetClientPtr(s_base* ptr);
  s_base* GetClientPtr();
  const std::string& GetUri();
};

#endif