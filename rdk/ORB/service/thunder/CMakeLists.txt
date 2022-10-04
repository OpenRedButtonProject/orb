##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the ORB License that can be found in the LICENSE file at
# the top level of this repository.
#
cmake_minimum_required(VERSION 2.8)

find_package(WPEFramework)

#set(PLUGIN_NAME SamplePlugin)
#set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
#set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation)

set(PLUGIN_NAME ORB)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})             # WPEFrameworkORB
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation) # WPEFrameworkORBImplementation

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

###############################################################################
## libWPEFrameworkORB In process part
###############################################################################

add_library(${MODULE_NAME} SHARED
  src/Module.cpp
  src/ORB.cpp
  src/ORBJsonRpc.cpp
)


target_include_directories(${MODULE_NAME} PUBLIC
  src
  #../../library/src/core
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
)


install(TARGETS ${MODULE_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/wpeframework/plugins
)


###############################################################################
## ORBImplementation library
###############################################################################

# Build the implementation that runs out-of-process behind a COM-RPC interface
add_library(${PLUGIN_IMPLEMENTATION} SHARED
  src/ORBImplementation.cpp
  src/Module.cpp
  src/ORBEventListenerImpl.cpp
)

add_dependencies(${PLUGIN_IMPLEMENTATION} ORBEngine)

target_include_directories(${PLUGIN_IMPLEMENTATION} PUBLIC
  src
  ../../library/src/core
)

set_target_properties(${PLUGIN_IMPLEMENTATION} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
)

target_link_libraries(${PLUGIN_IMPLEMENTATION} PRIVATE
  ${NAMESPACE}Plugins::${NAMESPACE}Plugins
  ORBCore
)

install(TARGETS
        ${PLUGIN_IMPLEMENTATION}
        DESTINATION
        lib/${STORAGE_DIRECTORY}/plugins)

write_config(${PLUGIN_NAME})

###############################################################################
## libORBClient
###############################################################################

add_library(ORBClient SHARED 
  src/ORBJsonRpcClient.cpp
  src/ORBComRpcClient.cpp
  ../../library/src/core/ORBGenericClient.h
  ../../library/src/core/api/ORBBrowserApi.h
)

target_include_directories(ORBClient PUBLIC
  src
  ../../library/src/core
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
