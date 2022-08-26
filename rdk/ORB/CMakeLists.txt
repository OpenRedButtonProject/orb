cmake_minimum_required(VERSION 2.8)

find_package(WPEFramework)

set(PLUGIN_NAME ORB)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

message(STATUS "==========================================================")
message(STATUS "${CMAKE_MODULE_PATH}")
message(STATUS "${NAMESPACE}")
message(STATUS "==========================================================")

set(PLUGIN_ORB_AUTOSTART "true" CACHE STRING "Automatically start ORB plugin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

find_path(XML2_INCLUDE_DIR NAMES parser.h PATH_SUFFIXES libxml2/libxml)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../components/application_manager ${CMAKE_CURRENT_BINARY_DIR}/application_manager)

# core sources
set(CORE_SOURCES
  src/core/ORBEvents.cpp
  src/core/ORBPlatformLoader.cpp
  src/core/SessionCallbackImpl.cpp
)

# core/dataTypes sources
set(CORE_DATA_TYPES_SOURCES
  src/core/dataTypes/URI.cpp
)

# core/requestHandlers sources
set(CORE_REQUEST_HANDLERS_SOURCES
  src/core/requestHandlers/BroadcastRequestHandler.cpp
  src/core/requestHandlers/ConfigurationRequestHandler.cpp
  src/core/requestHandlers/ManagerRequestHandler.cpp
  src/core/requestHandlers/ParentalControlRequestHandler.cpp
  src/core/requestHandlers/ProgrammeRequestHandler.cpp
)

# core/utilities sources
set(CORE_UTILITIES_SOURCES
  src/core/utilities/Base64.cpp
  src/core/utilities/HttpDownloader.cpp
  src/core/utilities/Keys.cpp
  src/core/utilities/MetadataSearchTask.cpp
  src/core/utilities/SHA256.cpp
  src/core/utilities/TokenManager.cpp
)

# platform sources
set(PLATFORM_SOURCES
  src/platform/dataTypes/Channel.cpp
  src/platform/dataTypes/Component.cpp
  src/platform/dataTypes/LocalSystem.cpp
  src/platform/dataTypes/ParentalRating.cpp
  src/platform/dataTypes/Programme.cpp
  src/platform/dataTypes/Query.cpp
)

# orb api headers
set(ORB_API_HEADERS
  src/platform/dataTypes/Channel.h
  src/platform/dataTypes/Component.h
  src/platform/dataTypes/LocalSystem.h
  src/platform/dataTypes/ParentalRating.h
  src/platform/dataTypes/Programme.h
  src/platform/dataTypes/Query.h
  src/platform/ORBEvents.h
  src/platform/ORBPlatform.h
)

add_library(${MODULE_NAME} SHARED
  ${CORE_SOURCES}
  ${CORE_DATA_TYPES_SOURCES}
  ${CORE_REQUEST_HANDLERS_SOURCES}
  ${CORE_UTILITIES_SOURCES}
  ${PLATFORM_SOURCES}
  src/Module.cpp
  src/ORB.cpp
  src/ORBJsonRpc.cpp
)

target_include_directories(${MODULE_NAME} PUBLIC
  src
  src/core
  ../../components/application_manager
  src/core/dataTypes
  src/core/requestHandlers
  src/core/utilities
  src/platform
  src/platform/dataTypes
  ${XML2_INCLUDE_DIR}/..
)

set_target_properties(${MODULE_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
)

target_compile_definitions(${MODULE_NAME} PUBLIC MODULE_NAME=${MODULE_NAME})

target_link_libraries(${MODULE_NAME} PRIVATE
  CompileSettingsDebug::CompileSettingsDebug
  ${NAMESPACE}Plugins::${NAMESPACE}Plugins
  ${NAMESPACE}Definitions::${NAMESPACE}Definitions
  applicationmanager
  curl
  uuid
  dl
)

install(TARGETS ${MODULE_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/wpeframework/plugins
)

install(FILES 
  ${ORB_API_HEADERS}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/include/orb
)

#add_subdirectory(src/platformMock)

write_config(${PLUGIN_NAME})
