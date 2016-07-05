set(PROTOCOL_SRC_LISTS 
  cpp/proto/global.Heartbeat.pb.cc
  cpp/proto/MessageHelper.cpp
  cpp/proto/dbproxy/dbproxy.Request.pb.cc
  cpp/proto/dbproxy/dbproxy.Response.pb.cc
)

include_directories(
  ${EDDY_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
  ${PROTOCOL_INCLUDE_DIRS}
)

link_directories(${BINARY_OUTPUT_DIR})

add_library(ProtocolShared SHARED
  ${PROTOCOL_SRC_LISTS}
)

add_library(ProtocolStatic STATIC
  ${PROTOCOL_SRC_LISTS}
)

set_target_properties(ProtocolShared
  PROPERTIES
  OUTPUT_NAME ${PROTOCOL_LIBRARY_NAME}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)

set_target_properties(ProtocolStatic
  PROPERTIES
  OUTPUT_NAME ${PROTOCOL_LIBRARY_NAME}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)