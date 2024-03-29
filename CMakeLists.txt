cmake_minimum_required(VERSION 3.13)
project (UMOSAPI)

find_package(libmongocxx REQUIRED)
find_package(libbsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})

find_package(nlohmann_json REQUIRED)
include_directories(${JSON_INCLUDE_DIR})

#find_package(Boost)

find_package(Threads)

find_path(JSONC_INCLUDE_DIRS json.h
                           HINTS
                           /usr/include/json-c/
                           /usr/local/include/json-c/
)
find_library(JSONC_LIBRARIES json-c
                          HINTS
                          /usr/lib/
                          /usr/local/lib
)

find_package_handle_standard_args(jsonc DEFAULT_MSG JSONC_INCLUDE_DIRS JSONC_LIBRARIES)

if (NOT JSONC_FOUND)
message(FATAL_ERROR "json-c lib not found! Please check if headers files are in
/usr/include/json-c/
or
/usr/local/include/json-c/
Also please check that libs files ares in
/usr/lib/
or
/usr/local/lib")
endif(NOT JSONC_FOUND)

find_path(RESTBED_INCLUDE_DIRS restbed
                           HINTS
                           /usr/include/corvusoft/
                           /usr/local/corvusoft/
)
find_library(RESTBED_LIBRARIES restbed
                          HINTS
                          /usr/lib/
                          /usr/local/lib
)

find_package_handle_standard_args(restbed DEFAULT_MSG RESTBED_INCLUDE_DIRS RESTBED_LIBRARIES)

if (NOT RESTBED_FOUND)
message(FATAL_ERROR "restbed lib not found! Please check if headers files are in
/usr/include/restbed/
or
/usr/local/include/restbed/
Also please check that libs files ares in
/usr/lib/
or
/usr/local/lib")
endif(NOT RESTBED_FOUND)

if(NOT TARGET spdlog)
    find_package(spdlog REQUIRED)
endif()

add_executable(umosapi main.cpp config.cpp api/umosapi.cpp db/mongo_access.cpp db/uobject.cpp api/umosapi.h config.h db/mongo_access.h db/uobject.h shared.h logging.h)

set_property(TARGET umosapi PROPERTY CXX_STANDARD 17)

target_include_directories(umosapi PUBLIC ${LIBMONGOCXX_INCLUDE_DIRS})
target_link_libraries(umosapi PUBLIC ${LIBBSONCXX_LIBRARIES})
target_link_libraries(umosapi PUBLIC ${LIBMONGOCXX_LIBRARIES})

target_link_libraries(umosapi PUBLIC ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(umosapi PUBLIC ${JSONC_INCLUDE_DIRS})
target_link_libraries(umosapi PUBLIC ${JSONC_LIBRARIES})

target_include_directories(umosapi PUBLIC ${RESTBED_INCLUDE_DIRS})
target_link_libraries(umosapi PUBLIC ${RESTBED_LIBRARIES})

target_link_libraries(umosapi PUBLIC spdlog::spdlog)
