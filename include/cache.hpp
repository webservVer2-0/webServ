#ifndef cache_hpp
#define cache_hpp

#include "cache.h"
#include "config.hpp"
#include "webserv.hpp"

typedef std::string extension;
typedef std::string mime_type;

typedef std::map<extension, mime_type> t_mime;
typedef std::pair<extension, mime_type> pair_mime;
typedef t_mime::iterator mime_iterator;

typedef std::string path;
typedef char* cache_entity;

typedef std::map<path, cache_entity> t_cache;

typedef struct s_loc t_loc;
typedef std::map<std::string, t_loc*> location_list;

void SetCache(t_server& target, t_cache& static_pages, t_cache& error_pages);
void SetStatic(location_list& location_configs, t_cache& static_pages);
void SetError(std::string& error, t_cache& error_pages);

void GetCacheList(t_cache& pages, t_cache& error);

#endif
