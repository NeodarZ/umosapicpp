#ifndef Uobject_H_
#define Uobject_H_

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

#include <json-c/json.h>

#include "../shared.h"
#include "../db/mongo_access.h"

namespace uobject {
    std::string retrieveAll(std::string collection, struct json_object* jsonObjects);
}

#endif
