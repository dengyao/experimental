set(TEST_PROJECT_NAME Test)

set(TEST_SRC_LISTS 
  Main.cpp
  proto/db.request.pb.cc
  proto/db.response.pb.cc
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
  ${EDDY_INCLUDE_DIRS}
  ${PROTOBUF_INCLUDE_DIRS}
)

link_directories(${BINARY_OUTPUT_DIR})

add_executable(${TEST_PROJECT_NAME} ${TEST_SRC_LISTS})

set_target_properties(${TEST_PROJECT_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
  "${BINARY_OUTPUT_DIR}"
)

target_link_libraries(${TEST_PROJECT_NAME}
  ${BASE_LIBRARY_NAME}
  ${EDDY_LIBRARY_NAME}
  ${MYSQL_LIBRARY_NAME}
  ${PROTOBUF_LIBRARY_NAME}
)