cmake_minimum_required(VERSION 3.4.1)
project(MockDialService)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(USE_DEBUG "debug" OFF)
option(USE_ANDROID_DEBUG "android debug" OFF)
if (CMAKE_CURRENT_BINARY_DIR MATCHES "debug")
   add_definitions(-DMGR_DEBUG)
   add_definitions(-DANDROID_DEBUG)
endif()

include_directories(
   ../../components/mock_dial_service/
)

find_library(log-lib log)
find_library(android-lib android)

add_library(
   org.orbtv.mockdialservice.mockdialservice STATIC
   ../../components/mock_dial_service/url_lib.c
   ../../components/mock_dial_service/dial_data.c
   ../../components/mock_dial_service/dial_server.c
   ../../components/mock_dial_service/quick_ssdp.c
   ../../components/mock_dial_service/mongoose.c
)

add_library(
   org.orbtv.mockdialservice.mockdialservice-jni SHARED
   # sources...
   src/main/cpp/mock_dial_service_native.cpp
   src/main/cpp/jni_utils.cpp
)

target_link_libraries(
   org.orbtv.mockdialservice.mockdialservice-jni
   # libraries...
   org.orbtv.mockdialservice.mockdialservice
   ${android-lib}
   ${log-lib}
)
