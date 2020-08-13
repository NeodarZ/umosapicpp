#ifndef SHARED_H_
#define SHARED_H_

#include <map>

#include "db/mongo_access.h"

extern std::map<std::string, std::string> config;
extern UmosapiService::mongo_access mongo;

#endif
