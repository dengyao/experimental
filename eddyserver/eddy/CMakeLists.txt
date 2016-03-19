set(EDDY_SRC
  eddy/IOServiceThread.cpp
  eddy/IOServiceThreadManager.cpp
  eddy/NetMessage.cpp
  eddy/SimpleMessageFilter.cpp
  eddy/TaskQueue.cpp
  eddy/TCPClient.cpp
  eddy/TCPServer.cpp
  eddy/TCPSession.cpp
  eddy/TCPSessionHandle.cpp
  eddy/TCPSessionQueue.cpp
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