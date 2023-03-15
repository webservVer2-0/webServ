#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

// int main() {
//   std::time_t now = std::time(nullptr);
//   std::stringstream ss;
//   ss << "Server received request at " << std::ctime(&now);
//   std::string time = ss.str();
//   std::cout << time;

//   std::chrono::seconds three_days(60 * 60 * 24 * 3);
//   std::chrono::system_clock::time_point future_time =
//       std::chrono::system_clock::from_time_t(now) + three_days;

//   // convert the future time to a string representation
//   std::time_t future_time_t =
//   std::chrono::system_clock::to_time_t(future_time); std::stringstream ns;

//   ns << "Future time: " << std::ctime(&future_time_t);
//   std::string future_time_string = ns.str();
//   std::cout << future_time_string;
// return 0;
// }

// int main() {
//   // get the current time
//   std::chrono::system_clock::time_point now =
//   std::chrono::system_clock::now();

//   // add 3 days to the current time
//   std::chrono::seconds three_days(3 * 24 * 60 * 60);  // 3 days in seconds
//   std::chrono::system_clock::time_point future_time = now + three_days;

//   // calculate the difference in days
//   auto difference =
//       std::chrono::duration_cast<std::chrono::seconds>(future_time - now);
//   int days_difference = difference.count() / 86400;  // 86400 seconds in a
//   day

//   std::cout << "Difference in days: " << days_difference << std::endl;

//   return 0;
// }

// #include <ctime>
// #include <iostream>

int main() {
  // get the current time
  std::time_t now = std::time(NULL);

  // format the current time as an HTTP date string
  char http_date[30];
  std::strftime(http_date, 30, "%a, %d %b %Y %H:%M:%S", std::localtime(&now));

  std::cout << "HTTP date: " << http_date << std::endl;

  return 0;
}
