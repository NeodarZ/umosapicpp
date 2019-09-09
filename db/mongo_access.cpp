#include "mongo_access.h"

UmosapiService::mongo_access::mongo_access(void) {};
UmosapiService::mongo_access::~mongo_access(void) {};

void UmosapiService::mongo_access::configure(mongocxx::uri uri) {
    class noop_logger : public mongocxx::logger {
        public:
            virtual void operator()(mongocxx::log_level,
                                    bsoncxx::stdx::string_view,
                                    bsoncxx::stdx::string_view) noexcept {}
    };

    UmosapiService::mongo_access::_pool = bsoncxx::stdx::make_unique<mongocxx::pool>(std::move(uri));
    UmosapiService::mongo_access::_instance = bsoncxx::stdx::make_unique<mongocxx::instance>(bsoncxx::stdx::make_unique<noop_logger>());
}
