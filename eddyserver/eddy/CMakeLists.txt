set(EDDY_SRC_LISTS
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
  ${EDDY_SRC_LISTS}
)

add_library(eddy_static STATIC
  ${EDDY_SRC_LISTS}
)

set_target_properties(eddy_shared
  PROPERTIES
  OUTPUT_NAME ${EDDY_LIBRARY_NAME}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)

set_target_properties(eddy_static
  PROPERTIES
  OUTPUT_NAME ${EDDY_LIBRARY_NAME}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)