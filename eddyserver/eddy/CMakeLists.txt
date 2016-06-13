set(EDDY_SRC
  eddy/IOServiceThread.cpp
  eddy/IOServiceThreadManager.cpp
  eddy/NetMessage.cpp
  eddy/DefaultMessageFilter.cpp
  eddy/TCPClient.cpp
  eddy/TCPServer.cpp
  eddy/TCPSession.cpp
  eddy/TCPSessionHandle.cpp
  eddy/TCPSessionQueue.cpp
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
)

add_library(eddy_shared SHARED
  ${EDDY_SRC}
)

add_library(eddy_static STATIC
  ${EDDY_SRC}
)

set_target_properties(eddy_shared
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