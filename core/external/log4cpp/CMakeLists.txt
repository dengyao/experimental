set(CURRENT_PROJECT_NAME Log4cpp)

set(CURRENT_PROJECT_SRC_LISTS 
  src/AbortAppender.cpp
  src/Appender.cpp
  src/AppendersFactory.cpp
  src/AppenderSkeleton.cpp
  src/BasicConfigurator.cpp
  src/BasicLayout.cpp
  src/BufferingAppender.cpp
  src/Category.cpp
  src/CategoryStream.cpp
  src/Configurator.cpp
  src/DllMain.cpp
  src/DummyThreads.cpp
  src/FactoryParams.cpp
  src/FileAppender.cpp
  src/Filter.cpp
  src/FixedContextCategory.cpp
  src/HierarchyMaintainer.cpp
  src/IdsaAppender.cpp
  src/LayoutAppender.cpp
  src/LayoutsFactory.cpp
  src/LevelEvaluator.cpp
  src/Localtime.cpp
  src/LoggingEvent.cpp
  src/Manipulator.cpp
  src/MSThreads.cpp
  src/NDC.cpp
  src/NTEventLogAppender.cpp
  src/OmniThreads.cpp
  src/OstreamAppender.cpp
  src/PassThroughLayout.cpp
  src/PatternLayout.cpp
  src/PortabilityImpl.cpp
  src/Priority.cpp
  src/Properties.cpp
  src/PropertyConfigurator.cpp
  src/PropertyConfiguratorImpl.cpp
  src/PThreads.cpp
  src/RemoteSyslogAppender.cpp
  src/RollingFileAppender.cpp
  src/SimpleConfigurator.cpp
  src/SimpleLayout.cpp
  src/SmtpAppender.cpp
  src/StringQueueAppender.cpp
  src/StringUtil.cpp
  src/SyslogAppender.cpp
  src/TimeStamp.cpp
  src/TriggeringEventEvaluatorFactory.cpp
  src/Win32DebugAppender.cpp
)

include_directories(
  ${LOG4CPP_INCLUDE_DIRS}
)

add_library(${CURRENT_PROJECT_NAME} ${COMPOLE_LIBRARY_TYPE}
  ${CURRENT_PROJECT_SRC_LISTS}
)

set_target_properties(${CURRENT_PROJECT_NAME}
  PROPERTIES
  OUTPUT_NAME ${LOG4CPP_LIBRARY_NAME}
  ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
  LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)