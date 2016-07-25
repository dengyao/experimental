set(CURRENT_PROJECT_NAME Protocol)

set(CURRENT_PROJECT_SRC_LISTS 
  code/ProtobufCodec.cpp
  code/proto/client_link.pb.cc
  code/proto/client_login.pb.cc
  code/proto/public_enum.pb.cc
  code/proto/public_struct.pb.cc
  code/proto/server_internal.pb.cc
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
  ${NETWORK_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
  ${PROTOCOL_INCLUDE_DIRS}
)

link_directories(${BINARY_OUTPUT_DIR})

add_library(${CURRENT_PROJECT_NAME} ${COMPOLE_LIBRARY_TYPE}
  ${CURRENT_PROJECT_SRC_LISTS}
)

set_target_properties(${CURRENT_PROJECT_NAME}
  PROPERTIES
  OUTPUT_NAME ${PROTOCOL_LIBRARY_NAME}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)