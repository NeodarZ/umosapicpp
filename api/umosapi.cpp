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

#include <regex>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <streambuf>

mongo_access mongo;


using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using json = nlohmann::json;

using namespace std;
using namespace restbed;

UmosapiService::UmosapiService() {}

void UmosapiService::init() {

    auto uri = mongocxx::uri{config["mongoURI"]};
    mongo.configure(std::move(uri));
    createResource();
    ofstream swagger_json;
    swagger_json.open(config["swaggerui"] + "/swagger.json");
    swagger_json << _swagger.dump();
    swagger_json.close();
}

void UmosapiService::start(int port, int thr) {
    auto settings = make_shared< Settings >();
    settings->set_port( port );
    settings->set_worker_limit( thr );
    settings->set_default_header("Connection", "close");
    _service.start(settings);
}

void service_error_handler( const int, const exception& e, const shared_ptr< Session > session )
{
    std::string message = "Backend Service is dead: ";
    message += e.what();
    if ( session->is_open( ) )
        session->close( 500, message, { { "Content-Length", ::to_string(message.length()) } } );
    fprintf( stderr, "ERROR: %s.\n", message.c_str() );
}

void resource_error_handler( const int, const exception& e, const shared_ptr< Session > session )
{
    std::string message = "Backend Resource is dead: ";
    message += e.what();
    if ( session->is_open( ) )
        session->close( 500, message, { { "Content-Length", ::to_string(message.length()) } } );
    fprintf( stderr, "ERROR: %s.\n", message.c_str() );
}

void faulty_method_handler( const shared_ptr< Session > )
{
    throw SERVICE_UNAVAILABLE;
}

void is_ready(const shared_ptr<Session> session)
{
    session->close( OK, "1", { { "Content-Length", "1"}});
}

/*
 * Add new route in service
 * service: the service pointer
 * route: path of the route
 * http_word: word http who defin REST request
 * callback: function to call when the route is requested
 * error_callback: error to show if everything is broken
 * tags: array of tags
 */
void UmosapiService::desc(std::string route, std::string http_word, const std::function< void ( const std::shared_ptr< Session > ) >& callback, const std::function< void(int, const std::exception&, std::shared_ptr<restbed::Session>) >& error_callback, tag tags[]) {
    auto resource = make_shared< Resource > ();
    resource->set_path(route);
    resource->set_method_handler(http_word, callback);
    resource->set_error_handler( error_callback );

    _service.publish(resource);
}

void UmosapiService::description(std::string description) {
    _swagger["info"]["description"] = description;
}

void UmosapiService::title(std::string title) {
    _swagger["info"]["title"] = title;
}

void UmosapiService::version(std::string version) {
    _swagger["info"]["version"] = version;
}

void UmosapiService::basePath(std::string basePath) {
    _swagger["swagger"] = "2.0";
    _swagger["basePath"] = basePath;
}

void UmosapiService::host(std::string host) {
    _swagger["host"] = host;
}

void UmosapiService::atag(std::string name, std::string description) {
    struct tag the_tag;
    the_tag.name = name;
    the_tag.description = description;

    _tags.push_back(the_tag);
    _swagger["tags"].push_back({ {"name",the_tag.name},{"description", the_tag.description} });

}

void UmosapiService::scheme(std::string scheme) {
    _swagger["schemes"].push_back(scheme);
}

void UmosapiService::set_path(std::string route) {
    _resource = make_shared< Resource > ();
    _resource->set_path(route);
    std::regex parameter(":.*?}");
    std::regex base_path("^/v2");
    auto tmp_route = std::regex_replace (route,parameter,"}");
    auto final_route = std::regex_replace (tmp_route,base_path,"");
    _path = Path{final_route};
    _swagger["paths"][final_route] = {};
}

void UmosapiService::set_method_handler(std::string http_word, const std::function< void ( const std::shared_ptr< Session > ) >& callback) {
    _resource->set_method_handler(http_word, callback);
    std::locale loc;
    for (auto& c : http_word) {
        c = tolower(c);
    }
    _path.words.push_back(HttpWord{http_word});
    _swagger["paths"][_path.name][http_word]["description"] = "";
    _swagger["paths"][_path.name][http_word]["operationId"] = "";
    _swagger["paths"][_path.name][http_word]["summary"] = "";
}

void UmosapiService::set_error_handler(const std::function< void(int, const std::exception&, std::shared_ptr<restbed::Session>) >& error_callback) {
    _resource->set_error_handler( error_callback );
}

void UmosapiService::publish() {
    for (auto& http_word: _path.words) {
        auto responses = _swagger["paths"][_path.name][http_word.name]["responses"];
        if (responses.find("200") == responses.end()) {
            _swagger["paths"][_path.name][http_word.name]["responses"]["200"]["description"] = "All is fine.";
        }
    }
    _service.publish(_resource);
}

void UmosapiService::definition(std::string name, std::string type) {
    _definition = Definition{name, type};
    _definitions.defs.push_back(_definition);
    _swagger["definitions"][name]["type"] = type;
}

void UmosapiService::propertie(std::string name, std::string format, std::string type, std::string required) {
    _definition.props.push_back(Propertie{name, format, type, required});
    _swagger["definitions"][_definition.name]["properties"][name]["format"] = format;
    _swagger["definitions"][_definition.name]["properties"][name]["type"] = type;
    if (required == "true") {
        _swagger["definitions"][_definition.name]["required"].push_back(name);
    }
}

void UmosapiService::consume(std::string consume) {
    _swagger["paths"][_path.name][_path.words.back().name]["consumes"].push_back(consume);
}

void UmosapiService::produce(std::string produce) {
    _swagger["paths"][_path.name][_path.words.back().name]["produces"].push_back(produce);
}

void UmosapiService::parameter(std::string name, std::string description, std::string schema = "") {
    json parameter;
    parameter["name"] = name;
    parameter["description"] = description;
    parameter["required"] = true;
    if (schema == "") {
        parameter["type"] = "string";
        parameter["in"] = "path";
    } else {
        parameter["in"] = "body";
        for (auto& def: _definitions.defs) {
            if (def.name == schema ) {
                std::string schema_path = "#/definitions/";
                parameter["schema"]["$ref"] = schema_path.append(schema);
            }
        }
    }
    _swagger["paths"][_path.name][_path.words.back().name]["parameters"].push_back(parameter);
}

void UmosapiService::response(std::string http_code, std::string description, std::string definition) {
    std::string schema = "#/definitions/";
    _swagger["paths"][_path.name][_path.words.back().name]["responses"][http_code]["description"] = description;
    _swagger["paths"][_path.name][_path.words.back().name]["responses"][http_code]["schema"]["items"]["$ref"] = schema.append(definition);
    _swagger["paths"][_path.name][_path.words.back().name]["responses"][http_code]["schema"]["type"] = "array";
}

void UmosapiService::swagger(std::string ui_path, std::string swagger_dir, std::string api_path) {
    _resource = make_shared< Resource > ();
    _resource->set_path(ui_path);
    _resource->set_method_handler("GET", swaggerEndpoint);
    _service.publish(_resource);

    _resource = make_shared< Resource > ();
    _resource->set_path(ui_path + "/{filename: .*}");
    _resource->set_method_handler("GET", swaggerEndpointResources);
    _service.publish(_resource);

    _resource = make_shared< Resource > ();
    _resource->set_path(api_path);
    _resource->set_method_handler("GET", swaggerEndpointApi);
    _service.publish(_resource);
}


void UmosapiService::createResource() {
    basePath("/v2");
    description("Umosapi rest api");
    version("0.1");
    title("UMOSAPI");
    host("127.0.0.1:"+config["port"]);

    swagger("/doc", config["swaggerui"], "/api");

    atag("uobject", "Everything about your UObjec");

    scheme("http");
    scheme("https");

    definition("UObject", "object");
    propertie("id", "int64", "integer", "true");
    propertie("value", "string", "object", "true");

    definition("UObjectSended", "object");

    set_path("/v2/ready");
    set_method_handler("GET", is_ready);
    produce("application/json");
    publish();

    set_path("/v2/{mcollection: .*}");
    set_method_handler("GET", retrieveAll);
    produce("application/json");
    parameter("mcollection", "Name of the collection where the uobject are located");
    set_error_handler(resource_error_handler);
    set_method_handler("POST", addUObject);
    consume("application/json");
    parameter("mcollection", "Name of the collection where the uobject are located");
    parameter("body", "UObject to add", "UObjectSended");
    set_error_handler(resource_error_handler);
    publish();

    set_path("/v2/{mcollection: .*}/{oid: .*}");
    set_method_handler("DELETE", deleteUObject);
    produce("application/json");
    parameter("mcollection", "Name of the collection where the uobject are located" );
    parameter("oid", "MongoDB oid of the uobject");
    set_error_handler(resource_error_handler);
    publish();

    set_path("/v2/{mcollection: .*}/{key: .*}/{value: .*}");
    set_method_handler("GET", searchUObjectByKeyValue);
    produce("application/json");
    parameter("mcollection", "Name of the collection where the uobject are located");
    parameter("key", "Key of uobject to search, ex: kill or total.kill");
    parameter("value", "Value of uobject to search, ex: 42");
    set_error_handler(resource_error_handler);
    publish();

    _service.set_error_handler( service_error_handler );
}

void UmosapiService::retrieveAll( const shared_ptr<Session> session ){
    auto jsonObjects = json_object_new_array();
    const auto& request = session->get_request( );
    auto json_string = uobject::retrieveAll(request->get_path_parameter( "mcollection" ), jsonObjects);
    json_object_put(jsonObjects);

    session->close( OK, json_string, {
        { "Content-Length", ::to_string(json_string.length()) },
        { "Content-Type", "application/json" } 
    });
}

void UmosapiService::addUObject( const shared_ptr<Session> session ){
    const auto request = session->get_request();

    size_t content_length = request->get_header( "Content-Length", 0 );

    session->fetch( content_length, [ request ]( const shared_ptr< Session > session, const Bytes & body )
    {
        auto jsonObject = json_object_new_object();
        char bodyData[body.size()+1];
        memset(bodyData, 0, sizeof(bodyData));
        snprintf(bodyData, sizeof(bodyData), "%.*s", ( int ) body.size( ), body.data());
        auto json_string = uobject::add(request->get_path_parameter("mcollection"), jsonObject, bodyData);
        session->close( OK, json_string, {
            { "Content-Length", ::to_string(json_string.length()) },
            { "Content-Type", "application/json" } 
        });
        json_object_put(jsonObject);
    } );
}

void UmosapiService::deleteUObject( const shared_ptr<Session> session ){

    auto jsonObject = json_object_new_object();
    const auto request = session->get_request();

    auto json_string = uobject::remove(request->get_path_parameter("mcollection"), request->get_path_parameter("oid"), jsonObject);

    session->close( OK, json_string, {
        { "Content-Length", ::to_string(json_string.length()) },
        { "Content-Type", "application/json" }
    });
    json_object_put(jsonObject);
}

void UmosapiService::searchUObjectByKeyValue( const shared_ptr<Session> session ){
    auto jsonObject = json_object_new_array();
    const auto request = session->get_request();

    auto json_string = uobject::searchKeyValue(request->get_path_parameter("mcollection"), request->get_path_parameter("key"), request->get_path_parameter("value"), jsonObject);

    session->close( OK, json_string, {
        { "Content-Length", ::to_string(json_string.length()) },
        { "Content-Type", "application/json" }
    });
    json_object_put(jsonObject);
}

void UmosapiService::swaggerEndpoint( const shared_ptr<Session> session ){
    const auto request = session->get_request();

    ifstream stream(config["swaggerui"] + "/index.html", ifstream::in );

    if ( stream.is_open() ) {
        const string body = string( istreambuf_iterator< char >(stream), istreambuf_iterator< char>());

        const multimap< string, string> headers {
            { "Content-Type", "text/html" },
            { "Content-Length", ::to_string( body.length() ) }
        };
        session->close(OK, body, headers);
    } else {
        session->close( NOT_FOUND );
    }
}

void UmosapiService::swaggerEndpointResources( const shared_ptr< Session > session )
{
    const auto request = session->get_request( );
    const string filename = request->get_path_parameter( "filename" );

    ifstream stream( config["swaggerui"] + "/" + filename, ifstream::in );

    if ( stream.is_open( ) )
    {
        const string body = string( istreambuf_iterator< char >( stream ), istreambuf_iterator< char >( ) );

        const multimap< string, string > headers
        {
            { "Content-Length", ::to_string( body.length( ) ) }
        };

        session->close( OK, body, headers );
    }
    else
    {
        session->close( NOT_FOUND );
    }
}

void UmosapiService::swaggerEndpointApi( const shared_ptr< Session > session )
{
    const auto request = session->get_request( );
    const string filename = request->get_path_parameter( "filename" );
    const string path = request->get_path();

    ifstream stream( config["swaggerui"] + "/swagger.json", ifstream::in );

    if ( stream.is_open( ) )
    {
        const string body = string( istreambuf_iterator< char >( stream ), istreambuf_iterator< char >( ) );

        const multimap< string, string > headers
        {
            { "Content-Type", "application/json" },
            { "Content-Length", ::to_string( body.length( ) ) }
        };

        session->close( OK, body, headers );
    }
    else
    {
        session->close( NOT_FOUND );
    }
}
