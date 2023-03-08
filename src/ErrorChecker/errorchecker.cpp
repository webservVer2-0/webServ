#include "../../include/errorchecker.hpp"

void CheckError(ServerConfig* config, struct kevent* curr_event) {
  s_base_type* temp = static_cast<s_base_type*>(curr_event->udata);
  if (temp->GetType() == SERVER) {
    return;
  } else {
    s_client_type* target;
    if (temp->GetType() == WORK) {
      target = static_cast<s_client_type*>(
          static_cast<s_work_type*>(temp)->GetClientPtr());
    } else {
      target = static_cast<s_client_type*>(temp);
    }
    if (target->GetErrorCode() == OK || target->GetErrorCode() == NO_ERROR)
      return;
    target->SetStage(ERR_FIN);
    PutErrorPage(target, target->GetConfig());
    ChangeEvents(config->change_list_, curr_event->ident, 0, EV_DELETE, 0, 0,
                 NULL);
    ChangeEvents(config->change_list_, target->GetFD(), EVFILT_READ, EV_DISABLE,
                 0, 0, NULL);
    ChangeEvents(config->change_list_, target->GetFD(), EVFILT_WRITE, EV_ENABLE,
                 0, 0, target);
    return;
  }
}

void PutErrorPage(s_client_type* client, const t_server& self_config) {
  if (client->GetResponse().entity_ != NULL)
    delete[] client->GetResponse().entity_;
  int error_code = client->GetErrorCode();
  std::ostringstream temp;
  temp << error_code;
  std::string val = temp.str();
  char* entity = strdup(self_config.error_pages_.at(val));
  if (!entity) {
    // TODO: error handling
  }
  client->GetResponse().entity_ = entity;
  client->GetResponse().entity_length_ = val.size();
  temp.clear();
  return;
}
