set(EDDY_SRC_LISTS
  eddy/IOServiceThread.cpp
  eddy/IOServiceThreadManager.cpp
  eddy/NetMessage.cpp
  eddy/DefaultMessageFilter.cpp
  eddy/TCPClient.cpp
  eddy/TCPServer.cpp
  eddy/TCPSession.cpp
  eddy/TCPSessionHandler.cpp
  eddy/TCPSessionQueue.cpp
)

include_directories(
  ${ASIO_INCLUDE_DIRS}
)

if(BUILD_SHARED_LIBS)
  add_library(Eddy SHARED
    ${EDDY_SRC_LISTS}
  )

  set_target_properties(Eddy
    PROPERTIES
    OUTPUT_NAME ${EDDY_LIBRARY_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  )
else()
  add_library(Eddy STATIC
    ${EDDY_SRC_LISTS}
  )

  set_target_properties(Eddy
    PROPERTIES
    OUTPUT_NAME ${EDDY_LIBRARY_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  )
endif()