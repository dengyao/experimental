set(CURRENT_PROJECT_NAME Network)

set(CURRENT_PROJECT_SRC_LISTS
  network/IOServiceThread.cpp
  network/IOServiceThreadManager.cpp
  network/NetMessage.cpp
  network/DefaultMessageFilter.cpp
  network/TCPClient.cpp
  network/TCPServer.cpp
  network/TCPSession.cpp
  network/TCPSessionHandler.cpp
  network/TCPSessionQueue.cpp
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
)

add_library(${CURRENT_PROJECT_NAME} ${COMPOLE_LIBRARY_TYPE}
  ${CURRENT_PROJECT_SRC_LISTS}
)

set_target_properties(${CURRENT_PROJECT_NAME}
  PROPERTIES
  OUTPUT_NAME ${NETWORK_LIBRARY_NAME}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)