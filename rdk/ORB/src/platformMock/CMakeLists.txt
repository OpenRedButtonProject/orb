cmake_minimum_required(VERSION 2.8)

find_package(${NAMESPACE}Plugins REQUIRED)

project(ORBPlatformImpl)

include_directories(
  ../platform
  ../platform/dataTypes
)

link_directories(ORBPlatformImpl
  /usr/lib
  /usr/lib/wpeframework/plugins
)

add_library(ORBPlatformImpl SHARED
  ORBPlatformImpl.cpp
)

add_dependencies(ORBPlatformImpl ORBPlatformApi)

target_link_libraries(ORBPlatformImpl PRIVATE
  WPEFrameworkORB
)

set_target_properties(ORBPlatformImpl PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED YES
  LINKER_LANGUAGE CXX
)

target_link_libraries(ORBPlatformImpl PRIVATE
  ${NAMESPACE}Plugins::${NAMESPACE}Plugins
)

# install(TARGETS ORBPlatformImpl
#   DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
#   COMPONENT ORBPlatformImpl
# )
