cmake_minimum_required(VERSION 3.4.1)
project(applicationManager)

option(USE_DEBUG "debug" OFF)
option(USE_ANDROID_DEBUG "android debug" OFF)
if (CMAKE_CURRENT_BINARY_DIR MATCHES "debug")
   add_definitions(-DMGR_DEBUG)
   add_definitions(-DANDROID_DEBUG)
endif()

add_definitions(-DORB_HBBTV_VERSION=${ORB_HBBTV_VERSION})
add_definitions(-DLWS_VERSION_4=1)

include_directories(
   build/prebuilts/libxml2/include
   build/prebuilts/libwebsockets/include
   ../../components/network_services
   ../../components/network_services/media_synchroniser
   ../../components/network_services/app2app
   ../../components/network_services/json_rpc
   ../../components/application_manager
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
   org.orbtv.orblibrary.applicationmanager STATIC
   # sources...
   ../../components/application_manager/application_manager.cpp
   ../../components/application_manager/utils.cpp
   ../../components/application_manager/ait.cpp
   ../../components/application_manager/xml_parser.cpp
   ../../components/application_manager/app.cpp
)
target_link_libraries(
   org.orbtv.orblibrary.applicationmanager
)

set(
    NETWORK_SERVICES_SOURCE_FILES
    "../../components/network_services/app2app/app2app_local_service.cpp"
    "../../components/network_services/app2app/app2app_remote_service.cpp"
    "../../components/network_services/media_synchroniser/media_synchroniser.cpp"
    "../../components/network_services/media_synchroniser/Correlation.cpp"
    "../../components/network_services/media_synchroniser/CorrelatedClock.cpp"
    "../../components/network_services/media_synchroniser/ClockBase.cpp"
    "../../components/network_services/media_synchroniser/SysClock.cpp"
    "../../components/network_services/media_synchroniser/ClockUtilities.cpp"
    "../../components/network_services/service_manager.cpp"
    "../../components/network_services/UdpSocketService.cpp"
    "../../components/network_services/websocket_service.cpp"
    "../../components/network_services/media_synchroniser/WallClockService.cpp"
    "../../components/network_services/media_synchroniser/Nullable.h"
    "../../components/network_services/media_synchroniser/ContentIdentificationService.cpp"
    "../../components/network_services/media_synchroniser/CSSUtilities.cpp"
    "../../components/network_services/media_synchroniser/TimelineSyncService.cpp"
)

if(DEFINED ORB_HBBTV_VERSION AND ORB_HBBTV_VERSION GREATER_EQUAL 204)
    list(
        APPEND NETWORK_SERVICES_SOURCE_FILES
        "../../components/network_services/json_rpc/JsonRpcService.h"
        "../../components/network_services/json_rpc/JsonRpcService.cpp"
    )
endif()

add_library(
    org.orbtv.orblibrary.networkservices STATIC
    ${NETWORK_SERVICES_SOURCE_FILES}
)

target_link_libraries(
   org.orbtv.orblibrary.networkservices
   openssl::ssl
   openssl::crypto
   jsoncpp::jsoncpp
   libwebsockets
)

set(
    NATIVE_SOURCE_FILES
    "src/main/cpp/jni_utils.cpp"
    "src/main/cpp/application_manager_native.cpp"
    "src/main/cpp/network_services_native.cpp"
    "src/main/cpp/native.cpp"
)

if(DEFINED ORB_HBBTV_VERSION AND ORB_HBBTV_VERSION GREATER_EQUAL 204)
    list(
        APPEND NATIVE_SOURCE_FILES
        "src/main/cpp/json_rpc_native.cpp"
    )
endif()

add_library(
    org.orbtv.orblibrary.native SHARED
    ${NATIVE_SOURCE_FILES}
)

target_link_libraries(
   org.orbtv.orblibrary.native
   # libraries...
   org.orbtv.orblibrary.networkservices
   org.orbtv.orblibrary.applicationmanager
   ${android-lib}
   ${log-lib}
   libxml2
)
