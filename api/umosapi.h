#ifndef UmosapiService_H_
#define UmosapiService_H_

#include <restbed>

#include <nlohmann/json.hpp>

namespace UmosapiService {

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

class Api {
    public:

        Api();
        virtual ~Api() {};

        void init();

        void start(int, int);
        restbed::Service _service;
        nlohmann::json _swagger;
        std::vector<tag> _tags;

    private:
        void desc(std::string route, std::string http_word, const std::function< void ( const std::shared_ptr< restbed::Session > ) >& callback, const std::function< void(int, const std::exception&, std::shared_ptr< restbed::Session >) >& error_callback, tag tags[]);
        void createResource();

        static void retrieveAll( const std::shared_ptr< restbed::Session > session );
        static void addUObject( const std::shared_ptr< restbed::Session > session );
        static void deleteUObject( const std::shared_ptr< restbed::Session > session );
        static void searchUObjectByKeyValue( const std::shared_ptr< restbed::Session > session );
        static void swaggerEndpoint( const std::shared_ptr< restbed::Session > session );
        static void swaggerEndpointResources( const std::shared_ptr< restbed::Session > session );
        static void swaggerEndpointApi( const std::shared_ptr< restbed::Session > session );

        std::shared_ptr< restbed::Resource> _resource;
        Definition _definition;
        Definitions _definitions;
        Path _path;
        Paths _paths;
        void set_path();
        void set_path(std::string route);
        void set_method_handler(std::string http_word, const std::function< void ( const std::shared_ptr< restbed::Session > ) >& callback);
        void set_error_handler(const std::function< void(int, const std::exception&, std::shared_ptr<restbed::Session>) >& error_callback);
        void produce(std::string);
        void consume(std::string);
        void parameter(std::string, std::string, std::string);
        void response(std::string err_code, std::string description, std::string schema, std::string type);
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
}
#endif
