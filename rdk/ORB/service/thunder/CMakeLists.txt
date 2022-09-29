##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the ORB License that can be found in the LICENSE file at
# the top level of this repository.
#
cmake_minimum_required(VERSION 2.8)

find_package(WPEFramework)

set(PLUGIN_NAME ORB)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

message(STATUS "==========================================================")
message(STATUS "${CMAKE_MODULE_PATH}")
message(STATUS "${NAMESPACE}")
message(STATUS "==========================================================")

set(PLUGIN_ORB_AUTOSTART "false" CACHE STRING "Automatically start ORB plugin")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(${NAMESPACE}Core REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

add_library(${MODULE_NAME} SHARED
  src/Module.cpp
  src/ORB.cpp
  src/ORBEventListenerImpl.cpp
  src/ORBJsonRpc.cpp
  src/ORBImplementation.cpp
  src/ORBComRpcClient.cpp
)

add_dependencies(${MODULE_NAME} ORBEngine)

target_include_directories(${MODULE_NAME} PUBLIC
  src
  ../../library/src/core
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
  ORBCore
)

install(TARGETS ${MODULE_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/wpeframework/plugins
)

write_config(${PLUGIN_NAME})

###############################################################################
## libORBClient
###############################################################################

add_library(ORBClient SHARED 
  src/ORBJsonRpcClient.cpp
  ../../library/src/core/ORBGenericClient.h
  ../../library/src/core/api/ORBBrowserApi.h
)

set_target_properties(ORBClient PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
)

target_compile_definitions(ORBClient PUBLIC MODULE_NAME=ORBClient)

target_include_directories(ORBClient PUBLIC
  src
  ../../library/src/core
  ../../library/src/core/api
)

target_link_libraries(ORBClient PRIVATE
  CompileSettingsDebug::CompileSettingsDebug
  ${NAMESPACE}Plugins::${NAMESPACE}Plugins
  ${NAMESPACE}Definitions::${NAMESPACE}Definitions
)

install(TARGETS ORBClient
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
