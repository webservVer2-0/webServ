#include "../../include/errorchecker.hpp"

void CheckError(struct kevent* curr_event) {
  s_client_type* target = static_cast<s_client_type*>(curr_event->udata);
  if (target == NULL) return;
  if (target->GetErrorCode() == OK || target->GetErrorCode() == NO_ERROR ||
      target->GetErrorCode() == MOV_PERMAN) {
    return;
  }
  PutErrorPage(target);
  ServerConfig::ChangeEvents(target->GetFD(), EVFILT_READ, EV_DISABLE, 0, 0,
                             NULL);
  ServerConfig::ChangeEvents(target->GetFD(), EVFILT_WRITE, EV_ENABLE, 0, 0,
                             target);
  return;
}

void PutErrorPage(s_client_type* client) {
  if (client->GetResponse().entity_length_ != 0)
    delete[] client->GetResponse().entity_;
  if (client->GetCacheError(client->GetErrorCode(), client->GetResponse()) ==
      false) {
    PrintError(4, WEBSERV, CRITICAL, "Error Cache Failed", "(PutErrorPage)");
  }
  return;
}
