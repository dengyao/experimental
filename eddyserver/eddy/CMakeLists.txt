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

add_library(eddy SHARED
  ${EDDY_SRC}
)

set_target_properties(eddy
  PROPERTIES
  OUTPUT_NAME "eddy"
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)