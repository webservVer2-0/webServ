#include "../include/webserv.hpp"

int main(int ac, char** av, char** en) {
  if (ac != 2) {
    SOUT << "Usage : ./webserv {config path}" << SEND;
    return (-1);
  }

  ServerConfig webserv(av[1]);

  webserv.PrintServerConfig();

  ServerInit(webserv);
  ServerBind(webserv);
  ServerListen(webserv);
  ServerKinit(webserv);

  //   for (int i = 0; i < webserv.GetServerNumber(); i++) {
  //     webserv.PrintTServer(i);
  //     std::cout << std::endl;
  //   }
  ServerRun(webserv);

  (void)en;
  // #if DG
  //   SOUT << "debug mode" << SEND;
  //   system("leaks webserv");
  // #else
  //   SOUT << "Not debug mode" << SEND;
  // #endif
  return (0);
}

// inline char* ChunkMsgFirst(char* send_size, std::string hex_size, t_http&
// res,
//                            size_t chunk_size) {
//   sprintf(send_size, "%s\r\n", hex_size);
//   char* buf = new char[sizeof(send_size) + chunk_size + 2];
//   std::memcpy(buf, send_size, sizeof(send_size));
//   std::memcpy(buf + sizeof(send_size), res.entity_, chunk_size);
//   std::memcpy(buf + sizeof(send_size) + chunk_size, "\r\n", 2);
//   return buf;
// }

// static char* Chunk_msg(s_client_type* client, char* msg) {
//   t_http res = client->GetResponse();
//   size_t chunk_size = static_cast<size_t>(
//       client->GetConfig().main_config_.find(BODY)->second.size());
//   std::string hex_size = to_hex_string(chunk_size);
//   std::string last_size = to_hex_string(chunk_size % res.entity_length_);
//   char* send_size;

//   if (client->GetChunkSize() == 0) {
//     client->SetStage(RES_CHUNK);
//     return msg;
//   }
//   if (client->GetType() == RES_CHUNK) {
//     if (client->GetChunkSize() == 0) {
//       return (ChunkMsgFirst(send_size, hex_size, res, chunk_size));
//     } else if (client->IncreaseChunked(chunk_size)) {
//       sprintf(send_size, "%s\r\n", hex_size);
//       char* buf = new char[sizeof(send_size) + chunk_size + 2];
//       std::memcpy(buf, send_size, sizeof(send_size));
//       std::memcpy(buf + sizeof(send_size), res.entity_ + chunk_size,
//                   chunk_size);
//       std::memcpy(buf + sizeof(send_size) + chunk_size, "\r\n", 2);
//       return buf;
//     } else {
//       sprintf(send_size, "%s\r\n", last_size);
//       char* buf = new char[sizeof(send_size) + chunk_size + 4];
//       std::memcpy(buf, send_size, sizeof(send_size));
//       std::memcpy(buf + sizeof(send_size), res.entity_, chunk_size);
//       std::memcpy(buf + sizeof(send_size) + chunk_size, "\r\n", 2);
//       client->SetStage(RES_FIN);
//       return buf;
//     }
//   }
//   sprintf(send_size, "0\r\n\r\n");
//   char* buf = new char[5];
//   std::memcpy(buf, send_size, sizeof(send_size));
//   return buf;
// }
