cmake_minimum_required(VERSION 2.8)

project(applicationmanager)

find_package(WPEFramework)

set(PLUGIN_NAME ORB)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})
find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)

find_path(XML2_INCLUDE_DIR NAMES parser.h PATH_SUFFIXES libxml2/libxml)

include_directories(
  ${XML2_INCLUDE_DIR}/..
  ../../rdk/ORB/src
)

link_libraries(
  CompileSettingsDebug::CompileSettingsDebug
  ${NAMESPACE}Plugins::${NAMESPACE}Plugins
  ${NAMESPACE}Definitions::${NAMESPACE}Definitions
  xml2
)

add_library(applicationmanager SHARED
  ait.cpp
  hbbtv_app.cpp
  opapp.cpp
  application_manager.cpp
  utils.cpp
  xml_parser.cpp
)

target_compile_definitions(applicationmanager PUBLIC MODULE_NAME=${MODULE_NAME})

set_target_properties(applicationmanager PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
)

install(TARGETS applicationmanager
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
)
