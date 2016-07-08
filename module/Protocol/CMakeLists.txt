set(PROTOCOL_SRC_LISTS 
  cpp/proto/externall.protocol.pb.cc
  cpp/proto/internal.protocol.pb.cc
  cpp/proto/MessageHelper.cpp
)

include_directories(
  ${EDDY_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
  ${PROTOCOL_INCLUDE_DIRS}
)

link_directories(${BINARY_OUTPUT_DIR})

if(BUILD_SHARED_LIBS)
  add_library(Protocol SHARED
    ${PROTOCOL_SRC_LISTS}
  )

  set_target_properties(Protocol
    PROPERTIES
    OUTPUT_NAME ${PROTOCOL_LIBRARY_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  )
else()
  add_library(Protocol STATIC
    ${PROTOCOL_SRC_LISTS}
  )
    
  set_target_properties(Protocol
    PROPERTIES
    OUTPUT_NAME ${PROTOCOL_LIBRARY_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  )
endif()