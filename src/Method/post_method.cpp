#include "../../include/webserv.hpp"

extern char** environ;  // TODO: 임시로 여기 적어 놓은거 헤더에 놓기

/**
 * @brief POST 작업을 할 파일이 텍스트 파일인지 아닌지 판단하는 함수입니다.
 *
 * @param client 클라이언트 데이터가 들어있는 s_client_type 구조체
 * @return true POST하려는 파일이 텍스트 파일인 경우
 * @return false POST하려는 파일이 텍스트 파일이 아닌 경우 (이미지 파일인 경우)
 */
bool IsTextFile(s_client_type* client) {
  std::string enctype;

  enctype = client->GetRequest().header_.find("Content-Type")->second;
  if (enctype == "application/x-www-form-urlencoded") {
    return true;
  } else {
    return false;
  }
}

/**
 * @brief 업로드를 위한 파일 경로를 생성하는 함수입니다.
 *
 * @param client 클라이언트 데이터가 들어있는 s_client_type 구조체
 * @return std::string 업로드 파일 경로
 * @note 업로드 파일 경로는 '{Cookie ID}_' 뒤에 텍스트 파일인 경우,
 * 'userdata.dat'이, 이미지 파일의 경우 'image.png'로 설정됩니다. 업로드 파일
 * 경로는 앞에 "./"가 붙습니다.
 */
std::string MakeFilePath(s_client_type* client) {
  std::string upload_path =
      client->GetConfig().main_config_.find("upload_path")->second;
  std::string ret = "./";

  ret.append(upload_path.append("/"));
  ret.append(client->GetCookieId());
  ret.push_back('_');
  if (IsTextFile(client)) {
    ret.append("userdata.dat");
  } else {
    ret.append("image.png");
  }
  return ret;
}

/**
 * @brief 기본 업로드 경로에 동일한 이름의 파일이 존재하는 경우, 파일명에
 * "(숫자)"를 붙여 저장할 파일 경로를 수정합니다.
 *
 * @param uri 파일 유형에 따른 기본 업로드 파일 경로
 * @return const char* 중복 여부에 따라 수정된 업로드 파일 경로
 * @note 함수 인자로 reference를 받습니다.
 * 이에 따라 기본 업로드 파일 경로도 중복 여부에 따라 반환값과 동일하게
 * 수정됩니다.
 *
 */
const char* AppendNumSuffix(std::string& uri) {
  std::stringstream file_suffix;
  size_t file_no = 1;
  size_t suffix_len;
  size_t suffix_pos;

  while (IsExistFile(uri.c_str())) {
    suffix_pos = uri.find('(');
    if (suffix_pos != std::string::npos) {
      uri.erase(suffix_pos, suffix_len);
    }
    file_suffix << "(" << file_no << ")";
    uri.insert(uri.rfind('.'), file_suffix.str());
    suffix_len = file_suffix.str().length();
    file_suffix.str("");
    file_no++;
  }
  return uri.c_str();
}

void ClientFilePost(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);

  std::string file_path = MakeFilePath(client);
  const char* path_char = AppendNumSuffix(file_path);

  int save_file_fd =
      open(path_char, O_RDWR | O_CREAT | O_APPEND | O_NONBLOCK, 0644);
  if (save_file_fd == -1) {
    client->SetErrorString(errno, "post_method.cpp / open()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(POST_FIN);
    return;
  }

  s_base_type* new_work = client->CreateWork(&(file_path), save_file_fd, file);
  ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                             client);
  ServerConfig::ChangeEvents(save_file_fd, EVFILT_WRITE, EV_ADD, 0, 0,
                             new_work);
  client->SetErrorCode(OK);
  client->SetStage(POST_START);
  return;
}

char* ExtractAsciiData(char* raw_entity, size_t entity_len) {
  char* data_start = strchr(raw_entity, '=') + 1;
  // TODO: = 뒤에 결과가 없을 경우 고려
  size_t data_len = (raw_entity + entity_len) - data_start;
  char* ascii_data = new char[data_len + 1];
  memmove(ascii_data, data_start, data_len);
  ascii_data[data_len] = '\0';
  return ascii_data;
}

void ClientCGIPost(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);

  char** args = new char*[4];
  args[0] = const_cast<char*>(client->GetConvertedURI().c_str());
  args[1] = const_cast<char*>(client->GetCookieId().c_str());
  args[2] = ExtractAsciiData(client->GetRequest().entity_,
                             client->GetRequest().entity_length_);
  args[3] = NULL;

  int cgi_pipe[2];
  if (pipe(cgi_pipe)) {
    // TODO: event disable?
    client->SetErrorString(errno, "post_method.cpp / pipe()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(POST_FIN);
    return;
  }
  pid_t child_pid = fork();
  if (child_pid == -1) {
    client->SetErrorString(errno, "post_method.cpp / fork()");
    client->SetErrorCode(SYS_ERR);
    client->SetStage(POST_FIN);
    return;
  } else if (child_pid == 0) {  // child process
    dup2(cgi_pipe[1], STDOUT_FILENO);
    close(cgi_pipe[1]);
    execve(args[0], args, environ);
    return;
  }
  close(cgi_pipe[1]);
  client->GetResponse().entity_length_ = cgi_pipe[0];
  ServerConfig::ChangeEvents(child_pid, EVFILT_PROC, EV_ADD, NOTE_EXIT, 0,
                             client);
  ServerConfig::ChangeEvents(client->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                             client);
  client->SetErrorCode(OK);
  client->SetStage(POST_START);
  return;
}

void ProcCGIPost(struct kevent* event) {
  s_client_type* client = static_cast<s_client_type*>(event->udata);
  int exit_status = event->data;
  int pipe_read = client->GetResponse().entity_length_;
  std::string cgi_path = client->GetConvertedURI();

  if (exit_status != 0) {
    // error
    client->SetErrorCode(SYS_ERR);
    close(pipe_read);  // close pipe;
  } else {
    // ok
    client->CreateWork(&cgi_path, pipe_read, cgi);
    client->SetErrorCode(OK);
    client->SetStage(POST_CGI);
  }
  return;
}

void WorkFilePost(struct kevent* event) {
  s_work_type* work = static_cast<s_work_type*>(event->udata);
  s_client_type* client = static_cast<s_client_type*>(work->GetClientPtr());

  ssize_t write_result = write(work->GetFD(), client->GetRequest().entity_,
                               client->GetRequest().entity_length_);
  if (write_result >= 0)
    client->SetMessageLength(client->GetMessageLength() +
                             static_cast<size_t>(write_result));
  size_t total_write_len = client->GetMessageLength();
  size_t entity_len = client->GetRequest().entity_length_;

  if (total_write_len < entity_len) {
    return;
  } else {
    work->ChangeClientEvent(EVFILT_WRITE, EV_ENABLE, 0, 0, client);
    ServerConfig::ChangeEvents(work->GetFD(), EVFILT_WRITE, EV_DELETE, 0, 0,
                               NULL);
    close(work->GetFD());
    std::string upload_path =
        client->GetConfig().main_config_.find("upload_path")->second;

    MakeDeletePage(client, client->GetResponse(), upload_path);

    client->SetMimeType(upload_path.append("/delete.html"));
    client->SetErrorCode(OK);
    client->SetStage(POST_FIN);
    client->SetMessageLength(0);
  }
  return;
}

void WorkCGIPost(struct kevent* event) {
  (void)event;
  return;
}
