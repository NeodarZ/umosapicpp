#ifndef MongoAccess_H_
#define MongoAccess_H_

#include <cstdlib>
#include <memory>

#include <bsoncxx/stdx/make_unique.hpp>
#include <bsoncxx/stdx/optional.hpp>
#include <bsoncxx/stdx/string_view.hpp>

#include <mongocxx/instance.hpp>
#include <mongocxx/logger.hpp>
#include <mongocxx/pool.hpp>
#include <mongocxx/uri.hpp>

#include <iostream>


class mongo_access {
    public:

        mongo_access ();
        ~mongo_access ();

        void configure(mongocxx::uri uri);

        using connection = mongocxx::pool::entry;

        connection get_connection() {
            return _pool->acquire();
        }

        bsoncxx::stdx::optional<connection> try_get_connection() {
            return _pool->try_acquire();
        }

    private:
        std::unique_ptr<mongocxx::instance> _instance = nullptr;
        std::unique_ptr<mongocxx::pool> _pool = nullptr;

};

#endif
