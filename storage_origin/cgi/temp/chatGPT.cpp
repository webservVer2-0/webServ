// #include <curl/curl.h>
// #include <unistd.h>

// #include <cstring>
// #include <iostream>

// #define MYKEY "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"

// struct WriteFunctionData {
//   std::string* stream;
// };

// size_t writeFunction(char* ptr, size_t size, size_t nmemb, void* userdata) {
//   auto data = reinterpret_cast<WriteFunctionData*>(userdata);
//   data->stream->append(ptr, size * nmemb);
//   return size * nmemb;
// }

// // ChatGPT API 엔드포인트와 API 키를 정의합니다.
// const std::string API_ENDPOINT =
//     "https://api.openai.com/v1/engines/davinci-codex/completions";
// const std::string API_KEY = MYKEY;

// // ChatGPT API에 HTTP POST 요청을 보내고, 결과를 문자열로 반환하는
// 함수입니다. std::string ask_chat_gpt(const std::string& question) {
//   // HTTP POST 요청을 보낼 CURL 핸들을 생성합니다.
//   CURL* curl = curl_easy_init();
//   if (!curl) {
//     return "";
//   }

//   // HTTP POST 요청에 필요한 데이터를 설정합니다.
//   std::string request_data =
//       "{\"prompt\":\"" + question + "\",\"max_tokens\":100}";
//   struct curl_slist* headers = NULL;
//   headers =
//       curl_slist_append(headers, ("Authorization: Bearer " +
//       API_KEY).c_str());
//   headers = curl_slist_append(headers, "Content-Type: application/json");
//   headers = curl_slist_append(headers, "User-Agent: curl");

//   // CURL 핸들을 이용해 HTTP POST 요청을 보냅니다.
//   std::string result;

//   curl_easy_setopt(curl, CURLOPT_URL, API_ENDPOINT.c_str());
//   curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
//   curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
//   // curl_easy_setopt 호출부
//   auto writeFunctionData = new WriteFunctionData{&result};
//   curl_easy_setopt(curl, CURLOPT_WRITEDATA, writeFunctionData);
//   //   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeFunction);
//   //   curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
//   CURLcode res = curl_easy_perform(curl);
//   curl_slist_free_all(headers);
//   curl_easy_cleanup(curl);

//   // 결과를 반환합니다.
//   if (res == CURLE_OK) {
//     return result;
//   } else {
//     return "";
//   }
// }

// int main() {
//   // 입력을 저장할 버퍼를 생성합니다.
//   char buffer[1024];
//   std::cin >> buffer;

//   // ChatGPT API에 질문을 보내고 결과를 받아옵니다.
//   std::string result = ask_chat_gpt(buffer);

//   // 결과를 출력합니다.
//   std::cout << "Content-Type: text/html\n\n";
//   std::cout << "<html><head><title>CGI Example</title></head><body>";
//   std::cout << "<h1>Result:</h1><p>" << result << "</p>";
//   std::cout << "</body></html>";

//   return 0;
// }

#include <curl/curl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

#define MYKEY "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"

struct WriteFunctionData {
  std::string* stream;
};

size_t writeFunction(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto data = reinterpret_cast<WriteFunctionData*>(userdata);
  data->stream->append(ptr, size * nmemb);
  return size * nmemb;
}

// ChatGPT API 엔드포인트와 API 키를 정의합니다.
const std::string API_ENDPOINT =
    "https://api.openai.com/v1/engines/davinci-codex/completions";
const std::string API_KEY = MYKEY;

// ChatGPT API에 HTTP POST 요청을 보내고, 결과를 문자열로 반환하는 함수입니다.
std::string ask_chat_gpt(const std::string& question) {
  // HTTP POST 요청을 보낼 CURL 핸들을 생성합니다.
  CURL* curl = curl_easy_init();
  if (!curl) {
    return "";
  }

  // HTTP POST 요청에 필요한 데이터를 설정합니다.
  std::string request_data =
      "{\"prompt\":\"" + question + "\",\"max_tokens\":100}";
  struct curl_slist* headers = NULL;
  headers =
      curl_slist_append(headers, ("Authorization: Bearer " + API_KEY).c_str());
  headers = curl_slist_append(headers, "Content-Type: application/json");
  headers = curl_slist_append(headers, "User-Agent: curl");

  // CURL 핸들을 이용해 HTTP POST 요청을 보냅니다.
  std::string result;

  curl_easy_setopt(curl, CURLOPT_URL, API_ENDPOINT.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_data.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  WriteFunctionData* writeFunctionData = new WriteFunctionData();
  writeFunctionData->stream = &result;
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, writeFunctionData);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeFunction);
  CURLcode res = curl_easy_perform(curl);
  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);

  // 결과를 반환합니다.
  if (res == CURLE_OK) {
    return result;
  } else {
    return "";
  }
}

int main() {
  // 입력을 저장할 버퍼를 생성합니다.
  std::string buffer;
  std::getline(std::cin, buffer);

  // ChatGPT API에 질문을 보내고 결과를 받아옵니다.
  std::string result = ask_chat_gpt(buffer);

  // 결과를 출력합니다.
  //   std::cout << "Content-Type: text/html\n\n";
  //   std::cout << "<html><head><title>CGI Example</title></head><body>";
  //   std::cout << "<h1>Result:</h1><p>" << result << "</p>";
  //   std::cout << "</body></html>";
  std::cout << "Result : " << result << std::endl;

  return 0;
}
