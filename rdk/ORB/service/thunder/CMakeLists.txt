##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
cmake_minimum_required(VERSION 2.8)

find_package(WPEFramework)

set(PLUGIN_NAME ORB)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})             # WPEFrameworkORB
set(PLUGIN_IMPLEMENTATION ${MODULE_NAME}Implementation) # WPEFrameworkORBImplementation

message(STATUS "==========================================================")
message(STATUS "${CMAKE_MODULE_PATH}")
message(STATUS "${NAMESPACE}")
message(STATUS "==========================================================")

if (NOT DEFINED PLUGIN_ORB_AUTOSTART)
  set(PLUGIN_ORB_AUTOSTART "false" CACHE STRING "Automatically start ORB plugin")
endif()

if (NOT DEFINED PLUGIN_ORB_PRIVATE_COMRPC)
  set(PLUGIN_ORB_PRIVATE_COMRPC "true" CACHE STRING "The plugin uses default communicator path (false) or private server (true)")  
endif()

add_definitions( -DPLUGIN_ORB_AUTOSTART=${PLUGIN_ORB_AUTOSTART} )
add_definitions( -DPLUGIN_ORB_PRIVATE_COMRPC=${PLUGIN_ORB_PRIVATE_COMRPC} )

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
  src/ORBComRpcServer.cpp
)

target_include_directories(${MODULE_NAME} PUBLIC
  src
  ../../library/src/core/utilities
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
        ${CMAKE_INSTALL_PREFIX}/lib/wpeframework/plugins)

write_config(${PLUGIN_NAME})

###############################################################################
## libORBClient
###############################################################################

add_library(ORBClient SHARED 
  src/ORBComRpcClient.cpp
  ../../library/src/core/ORBGenericClient.h
  ../../library/src/core/api/ORBBrowserApi.h
)

target_include_directories(ORBClient PUBLIC
  src
  ../../library/src/core/utilities
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
  ../../library/src/core/utilities
)

target_link_libraries(ORBClient PRIVATE
  CompileSettingsDebug::CompileSettingsDebug
  ${NAMESPACE}Plugins::${NAMESPACE}Plugins
  ${NAMESPACE}Definitions::${NAMESPACE}Definitions
)

install(TARGETS ORBClient
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)
