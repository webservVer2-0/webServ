#include <curl/curl.h>

#include <iostream>
#include <string>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                            void* userp) {
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

std::string ask_chat_gpt(const std::string& input_text) {
  std::string output_text;

  CURL* curl = curl_easy_init();
  if (curl) {
    // set the API endpoint URL
    std::string url =
        "https://api.openai.com/v1/engines/davinci-codex/completions";

    // set the request headers
    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(
        headers,
        "Authorization: Bearer "
        "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");

    // set the request body
    std::string data =
        "{\"prompt\": \"" + input_text + "\", \"max_tokens\": 150}";

    // set curl options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output_text);

    // perform the request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
      std::cerr << "Error performing request: " << curl_easy_strerror(res)
                << std::endl;
    }

    // free memory
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  } else {
    std::cerr << "Error initializing CURL" << std::endl;
  }

  // extract the response text
  size_t pos = output_text.find("\"text\": \"");
  if (pos != std::string::npos) {
    pos += 9;  // move past the text tag
    size_t end_pos = output_text.find("\"", pos);
    if (end_pos != std::string::npos) {
      output_text = output_text.substr(pos, end_pos - pos);
    }
  }

  return output_text;
}

int main() {
  std::string input_text;
  while (true) {
    std::cout << "You: ";
    std::getline(std::cin, input_text);

    if (input_text.empty()) {
      break;
    }

    std::string output_text = ask_chat_gpt(input_text);
    std::cout << "AI: " << output_text << std::endl;
  }

  return 0;
}
