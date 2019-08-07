#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

#include <json-c/json.h>

#include <nlohmann/json.hpp>

#include "umosapi.h"
#include "../shared.h"


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using json = nlohmann::json;

using namespace std;
using namespace Pistache;

namespace Generic {
    void handleReady(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "1");
    }
}

UmosapiService::UmosapiService(Address addr)
    : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
    , desc("Unity Mongo Save API", "0.1")
{ }

void UmosapiService::init(size_t thr = 2) {
    auto opts = Http::Endpoint::options()
        .threads(thr);
    httpEndpoint->init(opts);
    createDescription();
}

void UmosapiService::start(std::string swaggerui) {
    router.initFromDescription(desc);

    Rest::Swagger swagger(desc);
    swagger
        .uiPath("/doc")
        .uiDirectory(swaggerui)
        .apiPath("/api")
        .serializer(&Rest::Serializer::rapidJson)
        .install(router);

    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

void UmosapiService::createDescription() {
    desc
        .info()
        .license("Apache", "https://neodarz.net");

    auto backendErrorResponse =
        desc.response(Http::Code::Internal_Server_Error, "Backend is dead, it's the end of the world");

    desc
        .schemes(Rest::Scheme::Http)
        .basePath("/v1")
        .produces(MIME(Application, Json))
        .consumes(MIME(Application, Json));

    desc
        .route(desc.get("/ready"))
        .bind(&Generic::handleReady)
        .response(Http::Code::Ok, "Api started")
        .response(backendErrorResponse);

    auto versionPath = desc.path("/v1");

    versionPath
        .route(desc.get("/:mcollection"))
        .bind(&UmosapiService::UmosapiService::retrieveAll, this)
        .produces(MIME(Application, Json))
        .parameter<Rest::Type::String>("mcollection", "Name of the collection where the uobjects are located")
        .response(Http::Code::Ok, "List of uobjects")
        .response(backendErrorResponse);

    versionPath
        .route(desc.post("/:mcollection"))
        .bind(&UmosapiService::UmosapiService::addUObject, this)
        .produces(MIME(Application, Json))
        .parameter<Rest::Type::String>("mcollection", "Name of the collection where the uobjects are located")
        .response(Http::Code::Ok, "Uobject created")
        .response(backendErrorResponse);
}

void UmosapiService::retrieveAll(const Rest::Request& request, Http::ResponseWriter response) {
    mongocxx::client conn{mongocxx::uri{config["mongoURI"]}};

    auto collection = conn[config["mongo_db"]][request.param(":mcollection").as<string>()];

    auto cursor = collection.find({});

    auto jsonObjects = json_object_new_array();

    for (auto&& doc : cursor) {
        json_object_array_add(jsonObjects, json_tokener_parse(bsoncxx::to_json(doc).c_str()));
    }

    response.send(Http::Code::Ok, json_object_to_json_string_ext(jsonObjects, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY), MIME(Application, Json));
    json_object_put(jsonObjects);
}

void UmosapiService::addUObject(const Rest::Request& request, Http::ResponseWriter response) {
    mongocxx::client conn{mongocxx::uri{config["mongoURI"]}};

    auto collection = conn[config["mongo_db"]][request.param(":mcollection").as<string>()];

    auto document = bsoncxx::from_json(request.body().c_str());

    auto result = collection.insert_one(document.view());

    std::string oid = result->inserted_id().get_oid().value.to_string();

    auto jsonObject = json_object_new_object();

    int stringlen = 0;
    stringlen = strlen( request.body().c_str());
    struct json_tokener *tok = json_tokener_new();
    enum json_tokener_error jerr;

    json_object *json_datas = NULL;

    do {
        json_datas = json_tokener_parse_ex(tok, request.body().c_str(), stringlen);
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
        cout << typeid(json_datas).name() << endl;
        cout << "json_datas type: " << json_type_to_name(json_object_get_type(json_datas)) << endl;
    }
    json_tokener_reset(tok);

    response.send(Http::Code::Ok, json_object_to_json_string_ext(jsonObject, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY), MIME(Application, Json));
    json_object_put(jsonObject);
}
