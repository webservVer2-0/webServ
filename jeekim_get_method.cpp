#include "include/webserv.hpp"

//error_code 일단 다 sys_err로
//text file 기준으로 작성

t_error Open_File(t_error error_code)
{
  FILE* in_file = NULL;

  in_file = fopen("text file", "r");

  if (in_file == NULL) {
    std::cout << "Error: 읽기 실패" << std::endl;
    fclose(in_file);
    error_code = SYS_ERR; //sys_err?
  }
  return error_code;
}

t_error Get_File_Size(t_error error_code)
{
  if (fseek(in_file, 0, SEEK_END))
  {
    error_code = SYS_ERR;
    //바로 return ?
  }
  if ((buf_size = static_cast<size_t>(ftell(in_file))) == -1L)
  {
    error_code = SYS_ERR;
    //바로 return ?
  }

  rewind(in_file); // error 처리 해야함

  return error_code;
}

t_error Read_File(t_error error_code)
{
  char* buf = new char[buf_size];
  if (buf == NULL) {
    error_code = SYS_ERR;
  }

  result = fread(buf, sizeof(char), buf_size, in_file);
  if (result != buf_size) {
    std::cout << "Error: 읽기 " << std::endl;
    error_code = SYS_ERR;
  }

  return error_code;
}

t_error Get_Method(void* udata) {
  s_client_type* client_type = static_cast<s_client_type*>(udata);
  t_error error_code = NO_ERROR;

  switch (client_type->GetStage()) {
    case REQ_FIN:
      // open();
      if (error_code = Open_File())
      {
        //response에 보내야할것같은데 뭘 보내지?
      }
      else
      {
        Read_File();
      }
      break;
    case GET_READY:
      error_code = Read_File(); // SYS_ERR ?
      break;
    default:
      break;
  }


  return error_code;
}

#include <cstdio>
#include <iostream>

void binary_test() {
  FILE* in_file = NULL;
  size_t buf_size;
  size_t result;

  in_file = fopen("a.out", "rb");

  if (in_file == NULL) {
    std::cout << "Error: 읽기 실패" << std::endl;
    fclose(in_file);
    exit(1);
  }

  fseek(in_file, 0, SEEK_END);
  buf_size = static_cast<size_t>(ftell(in_file));
  rewind(in_file);

  char* buf = new char[buf_size];
  if (buf == NULL) {
    exit(1);
  }

  result = fread(buf, 1, buf_size, in_file);
  if (result != buf_size) {
    std::cout << "Error: 읽기 " << std::endl;
  }
  std::cout << "---bin---" << std::endl;
  std::cout << "bin size(): " << buf_size << std::endl;
  std::cout << "result: " << result << std::endl;

  std::cout << "---" << std::endl;

  FILE* out_file = fopen("ret_bin", "w");

  if (out_file == NULL) {
    std::cout << "Error: 파일 생성 실패" << std::endl;
  }
  size_t write_ret;

  write_ret = fwrite(buf, 1, buf_size, out_file);

  std::cout << "write_ret: " << write_ret << std::endl;

  fclose(out_file);
  fclose(in_file);
  delete[] buf;
}

void text_test() {
  FILE* in_file = NULL;
  size_t buf_size;
  size_t result;

  in_file = fopen("index.html", "r");

  if (in_file == NULL) {
    std::cout << "Error: 읽기 실패" << std::endl;
    fclose(in_file);
    exit(1);
  }

  fseek(in_file, 0, SEEK_END);
  buf_size = static_cast<size_t>(ftell(in_file));
  rewind(in_file);

  char* buf = new char[buf_size];
  if (buf == NULL) {
    exit(1);
  }

  result = fread(buf, sizeof(char), buf_size, in_file);
  if (result != buf_size) {
    std::cout << "Error: 읽기 " << std::endl;
  }
  std::cout << "---text---" << std::endl;
  std::cout << "bin size(): " << buf_size << std::endl;
  std::cout << "result: " << result << std::endl;

  std::cout << "---" << std::endl;

  FILE* out_file = fopen("ret_index", "w");

  if (out_file == NULL) {
    std::cout << "Error: 파일 생성 실패" << std::endl;
  }
  size_t write_ret;

  write_ret = fwrite(buf, sizeof(char), buf_size, out_file);

  std::cout << "write_ret: " << write_ret << std::endl;

  fclose(out_file);
  fclose(in_file);
  delete[] buf;
}

int main(void) {
  binary_test();
  text_test();
  return (0);
}
