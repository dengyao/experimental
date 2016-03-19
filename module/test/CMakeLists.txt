set(MODULE_NAME Main)

set(SRC_LISTS
  Main.cpp
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
  ${EDDY_INCLUDE_DIRS}
)

link_directories(${CMAKE_BINARY_DIR})

add_executable(${MODULE_NAME} ${SRC_LISTS})

set_target_properties(${MODULE_NAME}
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY
  "${CMAKE_BINARY_DIR}"
)

target_link_libraries(${MODULE_NAME}
  eddy
)