cmake_minimum_required(VERSION 3.13)
project (UMOSAPI)

find_package(libmongocxx REQUIRED)
find_package(libbsoncxx REQUIRED)
include_directories(${LIBMONGOCXX_INCLUDE_DIR})
include_directories(${LIBBSONCXX_INCLUDE_DIR})

find_package(Pistache REQUIRED)
include_directories(${Pistache_INCLUDE_DIR})

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


#add_executable(umosapi test.cpp)
add_executable(umosapi umosapi.cpp)

set_property(TARGET umosapi PROPERTY CXX_STANDARD 17)

target_include_directories(umosapi PUBLIC ${LIBMONGOCXX_INCLUDE_DIRS})
target_link_libraries(umosapi PUBLIC ${LIBBSONCXX_LIBRARIES})
target_link_libraries(umosapi PUBLIC ${LIBMONGOCXX_LIBRARIES})

target_link_libraries(umosapi PUBLIC ${Pistache_LIBRARIES}/libpistache.so)
target_include_directories(umosapi PUBLIC ${Pistache_INCLUDE_DIRS})

target_link_libraries(umosapi PUBLIC ${CMAKE_THREAD_LIBS_INIT})

target_include_directories(umosapi PUBLIC ${JSONC_INCLUDE_DIRS})
target_link_libraries(umosapi PUBLIC ${JSONC_LIBRARIES})
