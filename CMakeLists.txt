cmake_minimum_required(VERSION 2.8)

project(eddyserver CXX)

# 设置宏
if (WIN32)
add_definitions(-DASIO_MSVC)
endif()
add_definitions(-DASIO_STANDALONE)

# 设置编译选项
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")
endif()

# 设置类库名称
set(BASE_LIBRARY_NAME base)
set(EDDY_LIBRARY_NAME eddy)
set(PROTOBUF_LIBRARY_NAME protobuf)

if (WIN32)
set(MYSQL_LIBRARY_NAME libmysql)
else()
set(MYSQL_LIBRARY_NAME mysqlclient)
endif()

# 设置类库头文件包含目录
set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/asio)
set(BASE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/base)
set(EDDY_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/eddy)
set(PROTOBUF_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/external/protobuf)
set(RAPIDJSON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/external/rapidjson)
set(MYSQL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/external/mysql/include)
set(EASYLOGGINGPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/external/easyloggingpp)

# 设置二进制输出目录
set(BINARY_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 添加编译项目
add_subdirectory(eddyserver/base)
add_subdirectory(eddyserver/eddy)
add_subdirectory(eddyserver/external/protobuf)

add_subdirectory(module/DBProxy)
add_subdirectory(module/DBClient)