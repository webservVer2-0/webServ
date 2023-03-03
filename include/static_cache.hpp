#ifndef static_cache_hpp
#define static_cache_hpp

#include <sstream>

#include "webserv.hpp"

typedef std::map<std::string, char *> cache_data;
typedef std::map<std::string, size_t> cache_length;

typedef struct s_cache {
  cache_data data_collection_;
  cache_length length_collection_;
} t_cache;

#endif
