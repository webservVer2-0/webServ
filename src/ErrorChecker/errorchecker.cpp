#include "../../include/errorchecker.hpp"

void CheckError(struct kevent* curr_event) {
  s_base_type* temp = static_cast<s_base_type*>(curr_event->udata);
  if (temp == NULL) return;
  if (temp->GetType() == SERVER) {
    return;
  } else {
    s_client_type* target;
    if (temp->GetType() == LOGGER) return;
    if (temp->GetType() == WORK) {
      target = static_cast<s_client_type*>(
          static_cast<s_work_type*>(temp)->GetClientPtr());
    } else {
      target = static_cast<s_client_type*>(temp);
    }
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
}

void PutErrorPage(s_client_type* client) {
  if (client->GetResponse().entity_length_ != 0)
    delete[] client->GetResponse().entity_;
  client->GetCacheError(client->GetErrorCode(), client->GetResponse());
  return;
}
