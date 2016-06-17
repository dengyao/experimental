set(MODULE_NAME Main)

set(TEST_SRC_LISTS
  Main.cpp
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
  ${EDDY_INCLUDE_DIRS}
)

link_directories(${BINARY_OUTPUT_DIR})

add_executable(${MODULE_NAME} ${TEST_SRC_LISTS})

set_target_properties(${MODULE_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
  "${BINARY_OUTPUT_DIR}"
)

target_link_libraries(${MODULE_NAME}
  eddy
)