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

# install(TARGETS ${TARGET}
#    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/
#    COMPONENT ${TARGET}
# )
