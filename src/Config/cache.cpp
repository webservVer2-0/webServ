#include "../../include/cache.hpp"

#include "../../include/config.hpp"
#include "../../include/utils.hpp"

typedef std::string path;
typedef char* cache_entity;

typedef std::map<path, cache_entity> t_cache;
typedef std::pair<path, cache_entity> pair_cache;
typedef t_cache::iterator cache_iterator;

void SetCache(t_server& target, t_cache& static_pages, t_cache& error_pages) {
  SetStatic(target.location_configs_, static_pages);
  SetError(target.main_config_.at("error"), error_pages);
} // map.at()

void SetStatic(location_list& location_configs, t_cache& static_pages) {
  std::string file_path;
  location_list::iterator it = location_configs.begin();
  location_list::iterator end = location_configs.end();
  while (it != end) {
    if (it.operator->()->second->loc_type_[0] == T_ROOT) {
      config_map* temp = &(it.operator->()->second->main_config_);
      file_path += "./";
      file_path += temp->at(ROOT); //map
      file_path += "/";
      file_path += temp->at(DEFFILE);

      char* entity = GetFile(file_path.c_str());
      static_pages.insert(pair_cache(file_path, entity));

      file_path.clear();
    }
    it++;
  }
  (void)static_pages;
}

void SetError(std::string& error, t_cache& error_pages) {
  int i = 0;
  int limit = error.size();
  int pos = 0;
  std::string errnum;
  std::string value;

  while (i < limit) {
    if (isnumber(error.at(i))) {
      while (isnumber(error.at(i))) {
        value.push_back(error.at(i));
        i++;
      }
      if (error.at(i) == ' ') {
        errnum = value;
      }
      while (IsWhiteSpace(error.at(i))) {
        i++;
      }
      pos = i;
      while (!IsWhiteSpace(error.at(i))) {
        i++;
        if (i == limit) {
          break;
        }
      }
      value.clear();
      value = error.substr(pos, i - pos);
      std::string temp_path;
      temp_path.append("./");
      temp_path.append(value);

      char* entity = GetFile(temp_path.c_str());

      error_pages.insert(pair_cache(errnum, entity));

      temp_path.clear();
      value.clear();
      errnum.clear();
      while (i < limit && IsWhiteSpace(error.at(i))) {
        i++;
      }
    }
  }
}

void GetCacheList(t_cache& pages, t_cache& error) {
  t_cache::iterator pit = pages.begin();
  t_cache::iterator pend = pages.end();
  int i = 1;
  SOUT << "[ " << BOLDBLUE << std::setw(16) << ""
       << "static html pages " << RESET << std::setw(16) << " ]" << SEND;
  while (pit != pend) {
    SOUT << "[ " << GREEN << std::setw(15) << std::left << i << RESET << " : ";
    SOUT << std::setw(30) << std::right << pit.operator->()->first << " ]"
         << SEND;
    i++;
    pit++;
  }

  i = 1;
  t_cache::iterator eit = error.begin();
  t_cache::iterator eend = error.end();
  SOUT << "[ " << BOLDBLUE << std::setw(16) << ""
       << "static html pages " << RESET << std::setw(16) << " ]" << SEND;
  while (eit != eend) {
    SOUT << "[ " << GREEN << std::setw(15) << std::left << i << RESET << " : ";
    SOUT << std::setw(30) << std::right << eit.operator->()->first << " ]"
         << SEND;
    i++;
    eit++;
  }
}
