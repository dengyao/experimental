set(BASE_SRC_LISTS
  base/StringHelper.cpp
  base/TaskPools.cpp
  base/TaskQueue.cpp
)

if(BUILD_SHARED_LIBS)
  add_library(Base SHARED
    ${BASE_SRC_LISTS}
  )

  set_target_properties(Base
    PROPERTIES
    OUTPUT_NAME ${BASE_LIBRARY_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  )
else()
  add_library(Base STATIC
    ${BASE_SRC_LISTS}
  )

  set_target_properties(Base
    PROPERTIES
    OUTPUT_NAME ${BASE_LIBRARY_NAME}
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  )
endif()