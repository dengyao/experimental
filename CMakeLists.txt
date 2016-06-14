cmake_minimum_required(VERSION 2.8)

project(eddyserver CXX)

if (WIN32)
add_definitions(-DASIO_MSVC)
endif()
add_definitions(-DASIO_STANDALONE)

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread")
endif()

set(CMAKE_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(ASIO_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/asio)
set(EDDY_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/eddy)
set(PROTOBUF_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/eddyserver/protobuf)

add_subdirectory(eddyserver/eddy)
add_subdirectory(eddyserver/protobuf)
add_subdirectory(module/test)