#include "uobject.h"

std::string uobject::retrieveAll(std::string collection, struct json_object* jsonObjects) {

    auto conn = mongo.get_connection();

    auto coll = (*conn)[config["mongo_db"]][collection];

    auto cursor = coll.find({});

    for (auto&& doc : cursor) {
        json_object_array_add(jsonObjects, json_tokener_parse(bsoncxx::to_json(doc).c_str()));
    }

    return json_object_to_json_string_ext(jsonObjects, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
}
