set(CURRENT_PROJECT_NAME Common)

set(CURRENT_PROJECT_SRC_LISTS
  common/MD5.cpp
  common/Path.cpp
  common/CRC32.cpp
  common/TaskPools.cpp
  common/TaskQueue.cpp
  common/Singleton.cpp
  common/StringHelper.cpp
  common/ConfigManager.cpp
)

add_library(${CURRENT_PROJECT_NAME} ${COMPOLE_LIBRARY_TYPE}
  ${CURRENT_PROJECT_SRC_LISTS}
)

set_target_properties(${CURRENT_PROJECT_NAME}
	PROPERTIES
	OUTPUT_NAME ${COMMON_LIBRARY_NAME}
	ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
	LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)