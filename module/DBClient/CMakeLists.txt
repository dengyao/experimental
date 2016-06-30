set(DBCLIENT_PROJECT_NAME DBClient)

set(DBCLIENT_SRC_LISTS 
  Main.cpp
  DBClient.cpp
  proto/db.request.pb.cc
  proto/db.response.pb.cc
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
  ${EDDY_INCLUDE_DIRS}
  ${PROTO_INCLUDE_DIRS}
)

link_directories(${BINARY_OUTPUT_DIR})

add_executable(${DBCLIENT_PROJECT_NAME} ${DBCLIENT_SRC_LISTS})

set_target_properties(${DBCLIENT_PROJECT_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
  "${BINARY_OUTPUT_DIR}"
)

target_link_libraries(${DBCLIENT_PROJECT_NAME}
  ${BASE_LIBRARY_NAME}
  ${EDDY_LIBRARY_NAME}
  ${MYSQL_LIBRARY_NAME}
  ${PROTO_LIBRARY_NAME}
)