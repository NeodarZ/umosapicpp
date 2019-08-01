#include <string.h>
#include <filesystem>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <pistache/http.h>
#include <pistache/description.h>
#include <pistache/endpoint.h>

#include <pistache/serializer/rapidjson.h>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/stdx.hpp>
#include <mongocxx/uri.hpp>

#include <json-c/json.h>

#include "clara.hpp"

using namespace std;
using namespace Pistache;

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using namespace clara;

namespace Generic {
    void handleReady(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, "1");
    }
}

class UmosapiService {
    public:

        UmosapiService(Address addr)
            : httpEndpoint(std::make_shared<Http::Endpoint>(addr))
            , desc("Unity Mongo Save API", "0.1")
        { }

        void init(size_t thr = 2) {
            auto opts = Http::Endpoint::options()
                .threads(thr);
            httpEndpoint->init(opts);
            createDescription();
        }

        void start(std::string swaggerui) {
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

    private:
        void createDescription() {
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
                .bind(&UmosapiService::retrieveAll, this)
                .produces(MIME(Application, Json))
                .parameter<Rest::Type::String>("mcollection", "Name of the collection where the uobjects are located")
                .response(Http::Code::Ok, "List of uobjects")
                .response(backendErrorResponse);

        }

        void retrieveAll(const Rest::Request& request, Http::ResponseWriter response) {
            mongocxx::client conn{mongocxx::uri{"mongodb://127.0.0.1:27017"}};

            auto collection = conn["umosapi"][request.param(":mcollection").as<string>()];

            auto cursor = collection.find({});

            auto jsonObjects = json_object_new_array();

            for (auto&& doc : cursor) {
                json_object_array_add(jsonObjects, json_tokener_parse(bsoncxx::to_json(doc).c_str()));
            }

            response.send(Http::Code::Ok, json_object_to_json_string_ext(jsonObjects, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY), MIME(Application, Json));
            json_object_put(jsonObjects);
        }

        std::shared_ptr<Http::Endpoint> httpEndpoint;
        Rest::Description desc;
        Rest::Router router;
};

std::string get_value(struct json_object *jobj, const char *key) {
    struct json_object *tmp;


    json_object_object_get_ex(jobj, key, &tmp);

    if (jobj == NULL) {
        cout << "get_value: Json object is null" << endl;
        return "";
    } else {
        return json_object_get_string(tmp);
    }
}

int main(int argc, char *argv[]) {

    string config_path = "";
    const char *homedir;

    if ((homedir = getenv("XDG_CONFIG_HOME")) == NULL || (homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    config_path.append(homedir);
    config_path.append("/.config/umosapi/config.json");

    bool showHelp = false;
    int config_port = 9080;
    int thr = 2;
    auto cli = clara::detail::Help(showHelp)
             | clara::detail::Opt( config_path, "config" )["-c"]["--config"]("Config file path. Default ")
             | clara::detail::Opt( config_port, "port" )["-p"]["--port"]("Port to listen.")
             | clara::detail::Opt( thr, "treads" )["-t"]["--threads"]("Number of threads.");
    auto result = cli.parse( clara::detail::Args( argc, argv ) );
    if( !result )
    {
        std::cerr << "Error in command line: " << result.errorMessage() << std::endl;
        std::cerr << cli << std::endl;
        exit(1);
    }

    if ( showHelp ) {
        std::cerr << cli << std::endl;
        exit(1);
    }

    Address addr(Ipv4::any(), Port(config_port));

    cout << "Using " << hardware_concurrency() << " cores";
    cout << " - " << thr << " threads" << endl;
    cout << "Listen on 0.0.0.0:" << config_port << endl;

    cout << "Using config file '" << config_path << "'" << endl;

    if (!std::filesystem::exists(config_path)) {
        cout << "Error fatal : config file '" << config_path << "' not found" << endl;
        cout << "config.json is search here: ~/.config/umosapi/config.json" << endl;
        exit (EXIT_FAILURE);
    }

    auto config = json_object_from_file(config_path.c_str());

    //cout << get_value(config, "swaggerui") << endl;

    UmosapiService umosapi(addr);

    umosapi.init(thr);
    umosapi.start(get_value(config, "swaggerui"));
}
