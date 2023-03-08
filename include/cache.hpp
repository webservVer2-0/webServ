#ifndef cache_hpp
#define cache_hpp

#include <sstream>

#include "webserv.hpp"

typedef std::string path;
typedef char* cache_entity;

typedef std::map<path, cache_entity> t_cache;
typedef std::pair<path, cache_entity> pair_cache;
typedef t_cache::iterator cache_iterator;

#endif
