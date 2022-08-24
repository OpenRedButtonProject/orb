cmake_minimum_required(VERSION 3.4.1)
project(applicationManager)

option(USE_DEBUG "debug" OFF)
option(USE_ANDROID_DEBUG "android debug" OFF)
if (CMAKE_CURRENT_BINARY_DIR MATCHES "debug")
   add_definitions(-DMGR_DEBUG)
   add_definitions(-DANDROID_DEBUG)
endif()

include_directories(
   build/prebuilts/libxml2/include
   build/prebuilts/libwebsockets/include
   src/main/symlink.network_services
   src/main/symlink.network_services/media_synchroniser
   src/main/symlink.network_services/app2app
   src/main/symlink.application_manager
)

find_library(log-lib log)
find_library(android-lib android)

add_library(libxml2 STATIC IMPORTED)
add_library(libwebsockets STATIC IMPORTED)

find_package(openssl REQUIRED CONFIG)
find_package(jsoncpp REQUIRED CONFIG)

set_target_properties(
   libxml2 PROPERTIES
   IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/build/prebuilts/libxml2/distribution/${ANDROID_ABI}/liblibxml2.a
)

set_target_properties(
    libwebsockets PROPERTIES
    IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/build/prebuilts/libwebsockets/distribution/${ANDROID_ABI}/libwebsockets.a
)

add_library(
   org.orbtv.tvbrowser.applicationmanager STATIC
   # sources...
   src/main/symlink.application_manager/application_manager.cpp
   src/main/symlink.application_manager/utils.cpp
   src/main/symlink.application_manager/ait.cpp
   src/main/symlink.application_manager/xml_parser.cpp
   src/main/symlink.application_manager/app.cpp
)
target_link_libraries(
   org.orbtv.tvbrowser.applicationmanager
)

add_library(
   org.orbtv.tvbrowser.applicationmanager-jni SHARED
   # sources...
   src/main/cpp/application_manager_native.cpp
   src/main/cpp/jni_utils.cpp
)
target_link_libraries(
   org.orbtv.tvbrowser.applicationmanager-jni
   # libraries...
   org.orbtv.tvbrowser.applicationmanager
   ${android-lib}
   ${log-lib}
   libxml2
)

add_library(
        org.orbtv.tvbrowser.networkservices STATIC
        src/main/symlink.network_services/app2app/app2app_local_service.cpp
        src/main/symlink.network_services/app2app/app2app_remote_service.cpp
        src/main/symlink.network_services/media_synchroniser/media_synchroniser.cpp
        src/main/symlink.network_services/media_synchroniser/Correlation.cpp
        src/main/symlink.network_services/media_synchroniser/CorrelatedClock.cpp
        src/main/symlink.network_services/media_synchroniser/ClockBase.cpp
        src/main/symlink.network_services/media_synchroniser/SysClock.cpp
        src/main/symlink.network_services/media_synchroniser/ClockUtilities.cpp
        src/main/symlink.network_services/service_manager.cpp
        src/main/symlink.network_services/UdpSocketService.cpp
        src/main/symlink.network_services/websocket_service.cpp
        src/main/symlink.network_services/media_synchroniser/WallClockService.cpp
        src/main/symlink.network_services/media_synchroniser/Nullable.h
        src/main/symlink.network_services/media_synchroniser/ContentIdentificationService.cpp
        src/main/symlink.network_services/media_synchroniser/CSSUtilities.cpp
        src/main/symlink.network_services/media_synchroniser/TimelineSyncService.cpp
)
target_link_libraries(
        org.orbtv.tvbrowser.networkservices
        openssl::ssl
        openssl::crypto
        jsoncpp::jsoncpp
        libwebsockets
)

add_library(
        org.orbtv.tvbrowser.networkservices-jni SHARED
        # sources...
        src/main/cpp/network_services_native.cpp
        src/main/cpp/jni_utils.cpp
)
target_link_libraries(
        org.orbtv.tvbrowser.networkservices-jni
        # libraries...
        org.orbtv.tvbrowser.networkservices
        ${android-lib}
        ${log-lib}
)
