set(EDDY_SRC
  eddy/io_service_thread.cpp
  eddy/io_service_thread_manager.cpp
  eddy/message_filter.cpp
  eddy/net_message.cpp
  eddy/task_queue.cpp
  eddy/tcp_client.cpp
  eddy/tcp_server.cpp
  eddy/tcp_session.cpp
  eddy/tcp_session_handle.cpp
  eddy/tcp_session_queue.cpp
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
)

add_library(eddy_share SHARED
  ${EDDY_SRC}
)

add_library(eddy_static STATIC
  ${EDDY_SRC}
)

set_target_properties(eddy_share
  PROPERTIES
  OUTPUT_NAME "eddy"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

set_target_properties(eddy_static
  PROPERTIES
  OUTPUT_NAME "eddy"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)