project(eddyserver)

cmake_minimum_required(VERSION 2.8)

if (WIN32)
  add_definitions(-DASIO_MSVC)
endif()
add_definitions(-DASIO_STANDALONE)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -pthread")
endif()
message("CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})

if (WIN32)
  option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
else()
  option(BUILD_SHARED_LIBS "Build shared libraries" ON)
endif()

if(BUILD_SHARED_LIBS)
  set(COMPOLE_LIBRARY_TYPE SHARED)
else()
  set(COMPOLE_LIBRARY_TYPE STATIC)
endif()
set(BINARY_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

set(COMMON_LIBRARY_NAME common)
set(NETWORK_LIBRARY_NAME network)
set(PROTOBUF_LIBRARY_NAME protobuf)
set(PROTOCOL_LIBRARY_NAME protocol)
set(DBCLIENT_LIBRARY_NAME dbclient)
set(GWCLIENT_LIBRARY_NAME gwclient)

if (WIN32)
  set(MYSQL_LIBRARY_NAME libmysql)
else()
  set(MYSQL_LIBRARY_NAME mysqlclient)
endif()

set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/asio)
set(NETWORK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/network)
set(SPDLOG_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/spdlog)
set(PROTOBUF_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/protobuf)
set(RAPIDJSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/rapidjson)
set(MYSQL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/mysql/include)

set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/public/Common)
set(PROTOCOL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/public/Protocol/src)
set(DBCLIENT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/public/DBClient)
set(GWCLIENT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/public/GWClient)

add_subdirectory(core/network)
add_subdirectory(core/external/protobuf)

add_subdirectory(public/Common)
add_subdirectory(public/Protocol)
add_subdirectory(public/DBClient)
add_subdirectory(public/GWClient)

add_subdirectory(module/DBAgent)
add_subdirectory(module/GWServer)
add_subdirectory(module/LoginServer)
add_subdirectory(module/LinkerServer)