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
#include "../db/mongo_access.h"
#include "../db/uobject.h"

mongo_access mongo;


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
    auto uri = mongocxx::uri{config["mongoURI"]};
    mongo.configure(std::move(uri));
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
    auto jsonObjects = json_object_new_array();
    auto json_string = uobject::retrieveAll(request.param(":mcollection").as<string>(), jsonObjects);
    json_object_put(jsonObjects);

    response.send(Http::Code::Ok, json_string, MIME(Application, Json));
}

void UmosapiService::addUObject(const Rest::Request& request, Http::ResponseWriter response) {
    auto jsonObject = json_object_new_object();

    auto json_string = uobject::add(request.param(":mcollection").as<string>(), jsonObject,  request.body().c_str());

    response.send(Http::Code::Ok, json_string, MIME(Application, Json));
    json_object_put(jsonObject);
}
