##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the ORB License that can be found in the LICENSE file at
# the top level of this repository.
#
cmake_minimum_required(VERSION 2.8)

set(TARGET ORBCore)

add_definitions(-DRDK -DANDROID_DEBUG)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_path(XML2_INCLUDE_DIR NAMES parser.h PATH_SUFFIXES libxml2/libxml)

# core sources
set(CORE_SOURCES
   src/core/ORBEngine.cpp
   src/core/ORBPlatformEventHandlerImpl.cpp
   src/core/ORBPlatformLoader.cpp
   src/core/SessionCallbackImpl.cpp
)

# core/dataTypes sources
set(CORE_DATA_TYPES_SOURCES
   src/core/dataTypes/URI.cpp
)

# core/requestHandlers sources
set(CORE_REQUEST_HANDLERS_SOURCES
   src/core/requestHandlers/BroadcastRequestHandler.cpp
   src/core/requestHandlers/ConfigurationRequestHandler.cpp
   src/core/requestHandlers/ManagerRequestHandler.cpp
   src/core/requestHandlers/ParentalControlRequestHandler.cpp
   src/core/requestHandlers/ProgrammeRequestHandler.cpp
)

# core/utilities sources
set(CORE_UTILITIES_SOURCES
   src/core/utilities/Base64.cpp
   src/core/utilities/HttpDownloader.cpp
   src/core/utilities/JsonUtil.cpp
   src/core/utilities/MetadataSearchTask.cpp
   src/core/utilities/Query.cpp
   src/core/utilities/SHA256.cpp
   src/core/utilities/TokenManager.cpp
)

# orb api headers
set(ORB_API_HEADERS
   src/platform/dataTypes/Capabilities.h
   src/platform/dataTypes/Channel.h
   src/platform/dataTypes/Component.h
   src/platform/dataTypes/LocalSystem.h
   src/platform/dataTypes/ParentalRating.h
   src/platform/dataTypes/Programme.h
   src/platform/ORBPlatformEventHandler.h
   src/platform/ORBPlatform.h
)

add_library(${TARGET} SHARED
   ${CORE_SOURCES}
   ${CORE_DATA_TYPES_SOURCES}
   ${CORE_REQUEST_HANDLERS_SOURCES}
   ${CORE_UTILITIES_SOURCES}
)

target_include_directories(${TARGET} PUBLIC
   src
   src/core
   src/core/api
   src/core/dataTypes
   src/core/requestHandlers
   src/core/utilities
   src/platform
   src/platform/dataTypes
   src/thirdparty
   ${XML2_INCLUDE_DIR}/..
   ../../../components/application_manager
   )

set_target_properties(${TARGET} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED YES
   LINKER_LANGUAGE CXX
)


target_link_libraries(${TARGET} PRIVATE
   applicationmanager
   curl
   uuid
   dl
)

install(TARGETS ${TARGET}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
)

install(FILES 
   ${ORB_API_HEADERS}
   DESTINATION ${CMAKE_INSTALL_PREFIX}/include/orb
)

## application_manager

add_library(applicationmanager STATIC
   ../../../components/application_manager/ait.cpp
   ../../../components/application_manager/app.cpp
   ../../../components/application_manager/application_manager.cpp
   ../../../components/application_manager/utils.cpp
   ../../../components/application_manager/xml_parser.cpp
)

target_include_directories(applicationmanager PUBLIC
   ${XML2_INCLUDE_DIR}/..
)

target_link_libraries(applicationmanager PRIVATE
   xml2
)

set_target_properties(applicationmanager PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED YES
   LINKER_LANGUAGE CXX
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")