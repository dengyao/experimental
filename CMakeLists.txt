project(eddyserver)

cmake_minimum_required(VERSION 2.8)

# 设置宏
if (WIN32)
  add_definitions(-DASIO_MSVC)
endif()
add_definitions(-DASIO_STANDALONE)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")
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

# 设置类库名称
set(NETWORK_LIBRARY_NAME network)
set(PROTOBUF_LIBRARY_NAME protobuf)

set(COMMON_LIBRARY_NAME common)
set(PROTOCOL_LIBRARY_NAME protocol)
set(DBCLIENT_LIBRARY_NAME dbclient)
set(CONNECTOR_LIBRARY_NAME connector)

# 选择MySQL链接库
if (WIN32)
  set(MYSQL_LIBRARY_NAME libmysql)
else()
  set(MYSQL_LIBRARY_NAME mysqlclient)
endif()

# 设置类库头文件包含目录
set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/asio)
set(NETWORK_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/network)
set(PROTOBUF_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/protobuf)
set(RAPIDJSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/rapidjson)
set(MYSQL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/mysql/include)
set(EASYLOGGINGPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/core/external/easyloggingpp)

set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/module/Common)
set(PROTOCOL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/module/Protocol/code)
set(DBCLIENT_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/module/DBClient)
set(CONNECTOR_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/module/Connector)

# 设置二进制输出目录
set(BINARY_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 添加编译项目
add_subdirectory(core/network)
add_subdirectory(core/external/protobuf)

add_subdirectory(module/Common)
add_subdirectory(module/Protocol)
add_subdirectory(module/Router)
add_subdirectory(module/Connector)
add_subdirectory(module/DBAgent)
add_subdirectory(module/DBClient)