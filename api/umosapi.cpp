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

UmosapiService::mongo_access mongo;

using json = nlohmann::json;

UmosapiService::Api::Api() {}

void UmosapiService::Api::init() {

    auto uri = mongocxx::uri{config["mongoURI"]};
    mongo.configure(std::move(uri));
    UmosapiService::Api::createResource();
    ofstream swagger_json;
    swagger_json.open(config["swaggerui"] + "/swagger.json");
    swagger_json << UmosapiService::Api::_swagger.dump();
    swagger_json.close();
}

void UmosapiService::Api::start(int port, int thr) {
    auto settings = make_shared< restbed::Settings >();
    settings->set_port( port );
    settings->set_worker_limit( thr );
    settings->set_default_header("Connection", "close");
    UmosapiService::Api::_service.start(settings);
}

void service_error_handler( const int, const exception& e, const shared_ptr< restbed::Session > session )
{
    std::string message = "Backend Service is dead: ";
    message += e.what();
    if ( session ) {
        if ( session->is_open( ) ) {
            session->close( 500, message, { { "Content-Length", std::to_string(message.length()) } } );
        }
    }
    fprintf( stderr, "ERROR: %s.\n", message.c_str() );
}

void resource_error_handler( const int, const exception& e, const shared_ptr< restbed::Session > session )
{
    std::string message = "Backend Resource is dead: ";
    message += e.what();
    if ( session->is_open( ) )
        session->close( 500, message, { { "Content-Length", std::to_string(message.length()) } } );
    fprintf( stderr, "ERROR: %s.\n", message.c_str() );
}

void faulty_method_handler( const shared_ptr< restbed::Session > )
{
    throw SERVICE_UNAVAILABLE;
}

void is_ready(const shared_ptr< restbed::Session > session)
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
void UmosapiService::Api::desc(std::string route, std::string http_word, const std::function< void ( const std::shared_ptr< restbed::Session > ) >& callback, const std::function< void(int, const std::exception&, std::shared_ptr< restbed::Session >) >& error_callback, tag tags[]) {
    auto resource = make_shared< restbed::Resource > ();
    resource->set_path(route);
    resource->set_method_handler(http_word, callback);
    resource->set_error_handler( error_callback );

    UmosapiService::Api::_service.publish(resource);
}

void UmosapiService::Api::description(std::string description) {
    UmosapiService::Api::_swagger["info"]["description"] = description;
}

void UmosapiService::Api::title(std::string title) {
    UmosapiService::Api::_swagger["info"]["title"] = title;
}

void UmosapiService::Api::version(std::string version) {
    UmosapiService::Api::_swagger["info"]["version"] = version;
}

void UmosapiService::Api::basePath(std::string basePath) {
    UmosapiService::Api::_swagger["swagger"] = "2.0";
    UmosapiService::Api::_swagger["basePath"] = basePath;
}

void UmosapiService::Api::host(std::string host) {
    UmosapiService::Api::_swagger["host"] = host;
}

void UmosapiService::Api::atag(std::string name, std::string description) {
    struct tag the_tag;
    the_tag.name = name;
    the_tag.description = description;

    UmosapiService::Api::_tags.push_back(the_tag);
    UmosapiService::Api::_swagger["tags"].push_back({ {"name",the_tag.name},{"description", the_tag.description} });

}

void UmosapiService::Api::scheme(std::string scheme) {
    UmosapiService::Api::_swagger["schemes"].push_back(scheme);
}

void UmosapiService::Api::set_path(std::string route) {
    UmosapiService::Api::_resource = make_shared< restbed::Resource > ();
    UmosapiService::Api::_resource->set_path(route);
    std::regex parameter(":.*?}");
    std::regex base_path("^/v2");
    auto tmp_route = std::regex_replace (route,parameter,"}");
    auto final_route = std::regex_replace (tmp_route,base_path,"");
    UmosapiService::Api::_path = Path{final_route};
    UmosapiService::Api::_swagger["paths"][final_route] = {};
}

void UmosapiService::Api::set_method_handler(std::string http_word, const std::function< void ( const std::shared_ptr< restbed::Session > ) >& callback) {
    UmosapiService::Api::_resource->set_method_handler(http_word, callback);
    std::locale loc;
    for (auto& c : http_word) {
        c = tolower(c);
    }
    UmosapiService::Api::_path.words.push_back(HttpWord{http_word});
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][http_word]["description"] = "";
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][http_word]["operationId"] = "";
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][http_word]["summary"] = "";
}

void UmosapiService::Api::set_error_handler(const std::function< void(int, const std::exception&, std::shared_ptr< restbed::Session >) >& error_callback) {
    UmosapiService::Api::_resource->set_error_handler( error_callback );
}

void UmosapiService::Api::publish() {
    for (auto& http_word: UmosapiService::Api::_path.words) {
        auto responses = UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][http_word.name]["responses"];
        if (responses.find("200") == responses.end()) {
            UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][http_word.name]["responses"]["200"]["description"] = "All is fine.";
        }
    }
    UmosapiService::Api::_service.publish(_resource);
}

void UmosapiService::Api::definition(std::string name, std::string type) {
    UmosapiService::Api::_definition = Definition{name, type};
    UmosapiService::Api::_definitions.defs.push_back(_definition);
    UmosapiService::Api::_swagger["definitions"][name]["type"] = type;
}

void UmosapiService::Api::propertie(std::string name, std::string format, std::string type, std::string required) {
    UmosapiService::Api::_definition.props.push_back(Propertie{name, format, type, required});
    UmosapiService::Api::_swagger["definitions"][UmosapiService::Api::_definition.name]["properties"][name]["format"] = format;
    UmosapiService::Api::_swagger["definitions"][UmosapiService::Api::_definition.name]["properties"][name]["type"] = type;
    if (required == "true") {
        UmosapiService::Api::_swagger["definitions"][UmosapiService::Api::_definition.name]["required"].push_back(name);
    }
}

void UmosapiService::Api::consume(std::string consume) {
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][UmosapiService::Api::_path.words.back().name]["consumes"].push_back(consume);
}

void UmosapiService::Api::produce(std::string produce) {
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][UmosapiService::Api::_path.words.back().name]["produces"].push_back(produce);
}

void UmosapiService::Api::parameter(std::string name, std::string description, std::string schema = "") {
    json parameter;
    parameter["name"] = name;
    parameter["description"] = description;
    parameter["required"] = true;
    if (schema == "") {
        parameter["type"] = "string";
        parameter["in"] = "path";
    } else {
        parameter["in"] = "body";
        for (auto& def: UmosapiService::Api::_definitions.defs) {
            if (def.name == schema ) {
                std::string schema_path = "#/definitions/";
                parameter["schema"]["$ref"] = schema_path.append(schema);
            }
        }
    }
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][UmosapiService::Api::_path.words.back().name]["parameters"].push_back(parameter);
}

void UmosapiService::Api::response(std::string http_code, std::string description, std::string definition) {
    std::string schema = "#/definitions/";
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][UmosapiService::Api::_path.words.back().name]["responses"][http_code]["description"] = description;
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][UmosapiService::Api::_path.words.back().name]["responses"][http_code]["schema"]["items"]["$ref"] = schema.append(definition);
    UmosapiService::Api::_swagger["paths"][UmosapiService::Api::_path.name][UmosapiService::Api::_path.words.back().name]["responses"][http_code]["schema"]["type"] = "array";
}

void UmosapiService::Api::swagger(std::string ui_path, std::string swagger_dir, std::string api_path) {
    UmosapiService::Api::_resource = make_shared< restbed::Resource > ();
    UmosapiService::Api::_resource->set_path(ui_path);
    UmosapiService::Api::_resource->set_method_handler("GET", swaggerEndpoint);
    UmosapiService::Api::_service.publish(_resource);

    UmosapiService::Api::_resource = make_shared< restbed::Resource > ();
    UmosapiService::Api::_resource->set_path(ui_path + "/{filename: .*}");
    UmosapiService::Api::_resource->set_method_handler("GET", swaggerEndpointResources);
    UmosapiService::Api::_service.publish(_resource);

    UmosapiService::Api::_resource = make_shared< restbed::Resource > ();
    UmosapiService::Api::_resource->set_path(api_path);
    UmosapiService::Api::_resource->set_method_handler("GET", swaggerEndpointApi);
    UmosapiService::Api::_service.publish(_resource);
}


void UmosapiService::Api::createResource() {
    UmosapiService::Api::basePath("/v2");
    UmosapiService::Api::description("Umosapi rest api");
    UmosapiService::Api::version("0.1");
    UmosapiService::Api::title("UMOSAPI");
    UmosapiService::Api::host("127.0.0.1:"+config["port"]);

    UmosapiService::Api::swagger("/doc", config["swaggerui"], "/api");

    UmosapiService::Api::atag("uobject", "Everything about your UObjec");

    UmosapiService::Api::scheme("http");
    UmosapiService::Api::scheme("https");

    UmosapiService::Api::definition("UObject", "object");
    UmosapiService::Api::propertie("id", "int64", "integer", "true");
    UmosapiService::Api::propertie("value", "string", "object", "true");

    UmosapiService::Api::definition("UObjectSended", "object");

    UmosapiService::Api::set_path("/v2/ready");
    UmosapiService::Api::set_method_handler("GET", is_ready);
    UmosapiService::Api::produce("application/json");
    UmosapiService::Api::publish();

    UmosapiService::Api::set_path("/v2/{mcollection: .*}");
    UmosapiService::Api::set_method_handler("GET", retrieveAll);
    UmosapiService::Api::produce("application/json");
    UmosapiService::Api::parameter("mcollection", "Name of the collection where the uobject are located");
    UmosapiService::Api::set_error_handler(&resource_error_handler);
    UmosapiService::Api::set_method_handler("POST", addUObject);
    UmosapiService::Api::consume("application/json");
    UmosapiService::Api::parameter("mcollection", "Name of the collection where the uobject are located");
    UmosapiService::Api::parameter("body", "UObject to add", "UObjectSended");
    UmosapiService::Api::set_error_handler(&resource_error_handler);
    UmosapiService::Api::publish();

    UmosapiService::Api::set_path("/v2/{mcollection: .*}/{oid: .*}");
    UmosapiService::Api::set_method_handler("DELETE", deleteUObject);
    UmosapiService::Api::produce("application/json");
    UmosapiService::Api::parameter("mcollection", "Name of the collection where the uobject are located" );
    UmosapiService::Api::parameter("oid", "MongoDB oid of the uobject");
    UmosapiService::Api::set_error_handler(&resource_error_handler);
    UmosapiService::Api::publish();

    UmosapiService::Api::set_path("/v2/{mcollection: .*}/{key: .*}/{value: .*}");
    UmosapiService::Api::set_method_handler("GET", searchUObjectByKeyValue);
    UmosapiService::Api::produce("application/json");
    UmosapiService::Api::parameter("mcollection", "Name of the collection where the uobject are located");
    UmosapiService::Api::parameter("key", "Key of uobject to search, ex: kill or total.kill");
    UmosapiService::Api::parameter("value", "Value of uobject to search, ex: 42");
    UmosapiService::Api::set_error_handler(&resource_error_handler);
    UmosapiService::Api::publish();

    UmosapiService::Api::_service.set_error_handler( service_error_handler );
}

void UmosapiService::Api::retrieveAll( const shared_ptr< restbed::Session> session ){
    UmosapiService::uobject uobject;
    auto jsonObjects = json_object_new_array();
    const auto& request = session->get_request( );
    auto json_string = uobject.retrieveAll(request->get_path_parameter( "mcollection" ), jsonObjects);

    session->close( OK, json_string, {
        { "Content-Length", std::to_string(json_string.length()) },
        { "Content-Type", "application/json" } 
    });
    json_object_put(jsonObjects);

}

void UmosapiService::Api::addUObject( const shared_ptr< restbed::Session > session ){
    const auto request = session->get_request();

    size_t content_length = request->get_header( "Content-Length", 0 );

    session->fetch( content_length, [ request ]( const shared_ptr< restbed::Session > session, const Bytes & body )
    {
        UmosapiService::uobject uobject;
        auto jsonObject = json_object_new_object();
        char bodyData[body.size()+1];
        memset(bodyData, 0, sizeof(bodyData));
        snprintf(bodyData, sizeof(bodyData), "%.*s", ( int ) body.size( ), body.data());
        auto json_string = uobject.add(request->get_path_parameter("mcollection"), jsonObject, bodyData);
        session->close( OK, json_string, {
            { "Content-Length", ::to_string(json_string.length()) },
            { "Content-Type", "application/json" } 
        });
        json_object_put(jsonObject);
    } );
}

void UmosapiService::Api::deleteUObject( const shared_ptr< restbed::Session > session ){

    UmosapiService::uobject uobject;
    auto jsonObject = json_object_new_object();
    const auto request = session->get_request();

    auto json_string = uobject.remove(request->get_path_parameter("mcollection"), request->get_path_parameter("oid"), jsonObject);

    session->close( OK, json_string, {
        { "Content-Length", std::to_string(json_string.length()) },
        { "Content-Type", "application/json" }
    });
    json_object_put(jsonObject);
}

void UmosapiService::Api::searchUObjectByKeyValue( const shared_ptr< restbed::Session > session ){
    UmosapiService::uobject uobject;
    auto jsonObject = json_object_new_array();
    const auto request = session->get_request();

    auto json_string = uobject.searchKeyValue(request->get_path_parameter("mcollection"), request->get_path_parameter("key"), request->get_path_parameter("value"), jsonObject);

    session->close( OK, json_string, {
        { "Content-Length", std::to_string(json_string.length()) },
        { "Content-Type", "application/json" }
    });
    json_object_put(jsonObject);
}

void UmosapiService::Api::swaggerEndpoint( const shared_ptr< restbed::Session > session ){
    const auto request = session->get_request();

    ifstream stream(config["swaggerui"] + "/index.html", ifstream::in );

    if ( stream.is_open() ) {
        const std::string body = std::string( istreambuf_iterator< char >(stream), istreambuf_iterator< char>());

        const multimap< std::string, std::string> headers {
            { "Content-Type", "text/html" },
            { "Content-Length", std::to_string( body.length() ) }
        };
        session->close(OK, body, headers);
    } else {
        session->close( NOT_FOUND );
    }
}

void UmosapiService::Api::swaggerEndpointResources( const shared_ptr< restbed::Session > session )
{
    const auto request = session->get_request( );
    const std::string filename = request->get_path_parameter( "filename" );

    ifstream stream( config["swaggerui"] + "/" + filename, ifstream::in );

    if ( stream.is_open( ) )
    {
        const std::string body = std::string( istreambuf_iterator< char >( stream ), istreambuf_iterator< char >( ) );

        const multimap< std::string, std::string > headers
        {
            { "Content-Length", std::to_string( body.length( ) ) }
        };

        session->close( OK, body, headers );
    }
    else
    {
        session->close( NOT_FOUND );
    }
}

void UmosapiService::Api::swaggerEndpointApi( const shared_ptr< restbed::Session > session )
{
    const auto request = session->get_request( );
    const std::string filename = request->get_path_parameter( "filename" );
    const std::string path = request->get_path();

    ifstream stream( config["swaggerui"] + "/swagger.json", ifstream::in );

    if ( stream.is_open( ) )
    {
        const std::string body = std::string( istreambuf_iterator< char >( stream ), istreambuf_iterator< char >( ) );

        const multimap< std::string, std::string > headers
        {
            { "Content-Type", "application/json" },
            { "Content-Length", std::to_string( body.length( ) ) }
        };

        session->close( OK, body, headers );
    }
    else
    {
        session->close( NOT_FOUND );
    }
}
