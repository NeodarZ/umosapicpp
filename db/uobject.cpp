#include "uobject.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

std::string uobject::retrieveAll(std::string collection, struct json_object* jsonObjects) {

    auto conn = mongo.get_connection();

    auto coll = (*conn)[config["mongo_db"]][collection];

    auto cursor = coll.find({});

    for (auto&& doc : cursor) {
        json_object_array_add(jsonObjects, json_tokener_parse(bsoncxx::to_json(doc).c_str()));
    }

    return json_object_to_json_string_ext(jsonObjects, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
}

std::string uobject::add(std::string collection, struct json_object* jsonObject, const char * body) {
auto conn = mongo.get_connection();

    auto coll = (*conn)[config["mongo_db"]][collection];

    auto document = bsoncxx::from_json(body);

    auto result = coll.insert_one(document.view());

    std::string oid = result->inserted_id().get_oid().value.to_string();


    int stringlen = 0;
    stringlen = strlen( body );
    struct json_tokener *tok = json_tokener_new();
    enum json_tokener_error jerr;

    json_object *json_datas = NULL;

    do {
        json_datas = json_tokener_parse_ex(tok, body, stringlen);
    } while ((jerr = json_tokener_get_error(tok)) == json_tokener_continue);

    if (jerr != json_tokener_success)
    {
        fprintf(stderr, "Error: %s\n", json_tokener_error_desc(jerr));
    }

    if (json_object_get_type(json_datas) == json_type_object) {
        /* Create an object with the following template:
            * {
            *  "_id": {
            *      "$oid": "5d484d371ec4865f767d8424"
            *  },
            *  "datas": {
            *      [...]
            *  }
            * }
        */

        auto json_id = json_object_new_object();
        auto json_oid = json_object_new_string(oid.c_str());

        json_object_object_add(json_id, "$oid", json_oid);
        json_object_object_add(jsonObject, "_id", json_id);
        json_object_object_add(jsonObject, "datas", json_datas);
    } else {
        std::cout << typeid(json_datas).name() << std::endl;
        std::cout << "json_datas type: " << json_type_to_name(json_object_get_type(json_datas)) << std::endl;
    }
    json_tokener_reset(tok);

    return json_object_to_json_string_ext(jsonObject, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
}

std::string uobject::remove(std::string collection, std::string oid, struct json_object* jsonObject) {
    auto conn = mongo.get_connection();

    auto coll = (*conn)[config["mongo_db"]][collection];

    auto result = coll.delete_one(make_document(kvp("_id", bsoncxx::oid(oid))));


    if (result->deleted_count() > 0) {
        /* Create an object with the following template:
            * {
            *  "_id": {
            *      "$oid": "5d484d371ec4865f767d8424"
            *  },
            *  "datas": {
            *      [...]
            *  }
            * }
        */

        auto json_id = json_object_new_object();
        auto json_oid = json_object_new_string(oid.c_str());

        json_object_object_add(json_id, "$oid", json_oid);
        json_object_object_add(jsonObject, "_id", json_id);
        json_object_object_add(jsonObject, "datas", {});
    }
    return json_object_to_json_string_ext(jsonObject, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
}
