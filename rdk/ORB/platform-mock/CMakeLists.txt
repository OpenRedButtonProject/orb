##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the ORB License that can be found in the LICENSE file at
# the top level of this repository.
#
cmake_minimum_required(VERSION 2.8)

set(TARGET ORBPlatformImpl)

project(${TARGET})

add_library(${TARGET} SHARED
   src/DVB.cpp
   src/ORBPlatformMockImpl.cpp
)

target_include_directories(${TARGET} PRIVATE
   ../library/src/platform
   ../library/src/platform/dataTypes
   src
)

set_target_properties(${TARGET} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED YES
   LINKER_LANGUAGE CXX
)

install(TARGETS ${TARGET}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
   COMPONENT ${TARGET}
)
