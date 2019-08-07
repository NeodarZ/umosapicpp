#ifndef UmosapiService_H_
#define UmosapiService_H_

#include <pistache/http.h>
#include <pistache/description.h>
#include <pistache/endpoint.h>

#include <pistache/serializer/rapidjson.h>

using namespace Pistache;

class UmosapiService {
    public:

        UmosapiService(Address addr);
        virtual ~UmosapiService() {};

        void init(size_t thr);

        void start(std::string swaggerui);

    private:
        void createDescription();

        void retrieveAll(const Rest::Request& request, Http::ResponseWriter response);

        void addUObject(const Rest::Request& request, Http::ResponseWriter response);

        std::shared_ptr<Http::Endpoint> httpEndpoint;
        Rest::Description desc;
        Rest::Router router;
};
#endif
