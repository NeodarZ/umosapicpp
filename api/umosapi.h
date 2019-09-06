#ifndef UmosapiService_H_
#define UmosapiService_H_

//#include <pistache/http.h>
//#include <pistache/description.h>
//#include <pistache/endpoint.h>

#include <restbed>

//#include <pistache/serializer/rapidjson.h>

#include <nlohmann/json.hpp>

using namespace restbed;
using namespace std;

using json = nlohmann::json;

struct tag {
     std::string name;
     std::string description;
};

struct Propertie {

    std::string name;
    std::string format;
    std::string type;
    std::string required;
};

struct Definition {

    std::string name;
    std::string type;
    std::vector<Propertie> props;
};

struct Definitions {
    std::vector<Definition> defs;
};

struct HttpWord {
    std::string name;
};

struct Path {
    std::string name;
    std::vector<HttpWord> words;
};

struct Paths {
    std::vector<Path> paths;
};

class UmosapiService {
    public:

        UmosapiService();
        virtual ~UmosapiService() {};

        void init();

        void start(int, int);
        Service _service;
        json _swagger;
        vector<tag> _tags;

    private:
        void desc(std::string route, std::string http_word, const std::function< void ( const std::shared_ptr< Session > ) >& callback, const std::function< void(int, const std::exception&, std::shared_ptr<restbed::Session>) >& error_callback, tag tags[]);
        void createResource();

        static void retrieveAll( const shared_ptr<Session> session );
        static void addUObject( const shared_ptr<Session> session );
        static void deleteUObject( const shared_ptr<Session> session );
        static void searchUObjectByKeyValue( const shared_ptr<Session> session );
        static void swaggerEndpoint( const shared_ptr<Session> session );
        static void swaggerEndpointResources( const shared_ptr<Session> session );
        static void swaggerEndpointApi( const shared_ptr<Session> session );

        shared_ptr< Resource> _resource;
        Definition _definition;
        Definitions _definitions;
        Path _path;
        Paths _paths;
        void set_path();
        void set_path(std::string route);
        void set_method_handler(std::string http_word, const std::function< void ( const std::shared_ptr< Session > ) >& callback);
        void set_error_handler(const std::function< void(int, const std::exception&, std::shared_ptr<restbed::Session>) >& error_callback);
        void produce(std::string);
        void consume(std::string);
        void parameter(std::string, std::string, std::string);
        void response(std::string err_code, std::string description, std::string schema);
        void publish();
        void basePath(std::string basePath);
        void description(std::string description);
        void title(std::string title);
        void version(std::string version);
        void host(std::string host);
        void atag(std::string name, std::string description);
        void scheme(std::string scheme);

        void definition(std::string name, std::string type);
        void propertie(std::string name, std::string format, std::string type, std::string required);
        void swagger(std::string ui_path, std::string swagger_dir, std::string api_path);

};
#endif
