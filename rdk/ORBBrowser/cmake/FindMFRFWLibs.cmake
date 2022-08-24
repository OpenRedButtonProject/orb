# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# - Try to find MFRFWLIBS
# Once done this will define
#  MFRFWLIBS_FOUND - System has mfrfwlibs
#  MFRFWLIBS_INCLUDE_DIRS - The mfrfwlibs include directories
#  MFRFWLIBS_LIBRARIES - The libraries needed to use mfrfwlibs
#

find_package(PkgConfig)
pkg_check_modules(MFRFWLIBS fwupgrade)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(mfrfwlibs DEFAULT_MSG MFRFWLIBS_LIBRARIES)

mark_as_advanced(MFRFWLIBS_INCLUDE_DIRS MFRFWLIBS_LIBRARIES)


find_library(MFRFWLIBS_LIBRARY NAMES ${MFRFWLIBS_LIBRARIES}
        HINTS ${MFRFWLIBS_LIBDIR} ${MFRFWLIBS_LIBRARY_DIRS}
        )

if(MFRFWLIBS_LIBRARY AND NOT TARGET mfrfwlibs::mfrfwlibs)
    add_library(mfrfwlibs::mfrfwlibs UNKNOWN IMPORTED)
    set_target_properties(mfrfwlibs::mfrfwlibs PROPERTIES
            IMPORTED_LOCATION "${MFRFWLIBS_LIBRARY}"
            INTERFACE_LINK_LIBRARIES "${MFRFWLIBS_LIBRARIES}"
            INTERFACE_COMPILE_OPTIONS "${MFRFWLIBS_DEFINITIONS}"
            INTERFACE_INCLUDE_DIRECTORIES "${MFRFWLIBS_INCLUDE_DIRS}"
            )
endif()
