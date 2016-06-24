set(MODULE_NAME DBProxy)

set(DBPROXY_SRC_LISTS 
  Main.cpp
  TaskPools.cpp
  TaskQueue.cpp
)

include_directories(
  .
  ${ASIO_INCLUDE_DIRS}
  ${EDDY_INCLUDE_DIRS}
  ${MYSQL_INCLUDE_DIRS}
  ${RAPIDJSON_INCLUDE_DIRS}
  ${EASYLOGGINGPP_INCLUDE_DIRS}
)

link_directories(${BINARY_OUTPUT_DIR})

add_executable(${MODULE_NAME} ${DBPROXY_SRC_LISTS})

set_target_properties(${MODULE_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
  "${BINARY_OUTPUT_DIR}"
)

target_link_libraries(${MODULE_NAME}
  ${EDDY_LIBRARY_NAME}
  ${MYSQL_LIBRARY_NAME}
)