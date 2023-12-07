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
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# provide ORB_HBBTV_VERSION through recipe for 204
set(ORB_HBBTV_VERSION_VALUE 203)
if (DEFINED ORB_HBBTV_VERSION)
   set(ORB_HBBTV_VERSION_VALUE ${ORB_HBBTV_VERSION})
endif()
add_definitions(-DORB_HBBTV_VERSION=${ORB_HBBTV_VERSION_VALUE})

add_subdirectory(service/thunder)
add_subdirectory(library)
add_subdirectory(platform-mock)
