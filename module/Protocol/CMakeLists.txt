set(CURRENT_PROJECT_NAME 
  cpp/proto/externall.protocol.pb.cc
  cpp/proto/internal.protocol.pb.cc
  cpp/proto/MessageHelper.cpp
)

set(CURRENT_PROJECT_NAME 
  cpp/proto/externall.protocol.pb.cc
  cpp/proto/internal.protocol.pb.cc
  cpp/proto/MessageHelper.cpp
)

include_directories(
  ${PROTOBUF_INCLUDE_DIRS}
  ${PROTOCOL_INCLUDE_DIRS}
  ${EDDYSERVER_INCLUDE_DIRS}
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