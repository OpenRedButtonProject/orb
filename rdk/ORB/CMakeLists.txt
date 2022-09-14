##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the ORB License that can be found in the LICENSE file at
# the top level of this repository.
#
cmake_minimum_required(VERSION 2.8)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subdirectory(service/thunder)
add_subdirectory(library)
#add_subdirectory(platform-mock)