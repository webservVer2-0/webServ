#ifndef MIME_HPP
#define MIME_HPP

#include "webserv.hpp"

typedef std::string extension;
typedef std::string mime_type;

typedef std::map<extension, mime_type> t_mime;
typedef std::pair<extension, mime_type> pair_mime;
typedef t_mime::iterator mime_iterator;

void SetMime(t_mime& storage, std::string filepath);

#endif
