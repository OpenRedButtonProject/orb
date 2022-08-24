##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the ORB License that can be found in the LICENSE file at
# the top level of this repository.
#
cmake_minimum_required(VERSION 3.3)

find_package(WPEFramework)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake/")

include(CmakeHelperFunctions)

# Library installation section
string(TOLOWER ${NAMESPACE} STORAGE_DIRECTORY)

if(BUILD_REFERENCE)
    add_definitions(-DBUILD_REFERENCE=${BUILD_REFERENCE})
endif()

set(PLUGIN_NAME ORBBrowser)
set(MODULE_NAME ${NAMESPACE}${PLUGIN_NAME})

option(PLUGIN_ORBBROWSER_ENABLE_JIT "Enable the use of JIT javascript optimalization" ON)
option(PLUGIN_ORBBROWSER_ENABLE_DFG "Enable the use of DFG javascript optimalization" ON)

set(PLUGIN_ORBBROWSER_AUTOSTART false CACHE STRING "Automatically start WebKitBrowser plugin")
set(PLUGIN_ORBBROWSER_TRANSPARENT false CACHE STRING "Set transparency")
set(PLUGIN_ORBBROWSER_STARTURL "about:blank" CACHE STRING "Initial URL for WebKitBrowser plugin")
set(PLUGIN_ORBBROWSER_USERAGENT "" CACHE STRING "User agent string")
set(PLUGIN_ORBBROWSER_MEMORYPROFILE "512m" CACHE STRING "Memory Profile")
set(PLUGIN_ORBBROWSER_MEMORYPRESSURE "databaseprocess:50m,networkprocess:100m,webprocess:300m,rpcprocess:50m" CACHE STRING "Memory Pressure")
set(PLUGIN_ORBBROWSER_MEDIA_CONTENT_TYPES_REQUIRING_HARDWARE_SUPPORT "video/*" CACHE STRING "Media content types requiring hardware support")
set(PLUGIN_ORBBROWSER_MEDIADISKCACHE false CACHE STRING "Media Disk Cache")
set(PLUGIN_ORBBROWSER_MSEBUFFERS "audio:2m,video:15m,text:1m" CACHE STRING "MSE Buffers for WebKit")
set(PLUGIN_ORBBROWSER_DISKCACHE "0" CACHE STRING "Disk Cache")
set(PLUGIN_ORBBROWSER_XHRCACHE true CACHE STRING "XHR Cache")
set(PLUGIN_ORBBROWSER_LOCALSTORAGE "" CACHE STRING "HTML5 local storage path")
set(PLUGIN_ORBBROWSER_COOKIESTORAGE "" CACHE STRING "Browser cookie storage path")
set(PLUGIN_ORBBROWSER_WINDOWCLOSE false CACHE STRING "Allow window close")
set(PLUGIN_ORBBROWSER_WEBGL true CACHE STRING "Enable WebGL")
set(PLUGIN_ORBBROWSER_RESOLUTION "720p" CACHE STRING "Browser resolution")
set(PLUGIN_ORBBROWSER_THREADEDPAINTING "1" CACHE STRING "Threads for the Threaded Painting")
set(PLUGIN_ORBBROWSER_WEBINSPECTOR_ADDRESS 0.0.0.0:9225 CACHE STRING "IP:Port for WebInspector of WebKitBrowser")
set(PLUGIN_ORBBROWSER_LOCALSTORAGE_ENABLE true CACHE STRING "Enable LocalStorage of WebKitBrowser")
set(PLUGIN_ORBBROWSER_THUNDER_DECRYPTOR_PREFERENCE true CACHE STRING "Enable Thunder decryptor preference in WebKit")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(CompileSettingsDebug CONFIG REQUIRED)
find_package(GLIB REQUIRED)
find_package(WPEWebKit REQUIRED)
find_package(WPEBackend REQUIRED)
if(DEFINED WEBKIT_GLIB_API)
find_package(LibSoup REQUIRED)
endif()

set(SOURCE_LIST
    Module.cpp
    ORBBrowser.cpp
    ORBBrowserJsonRpc.cpp
    WebKitImplementation.cpp
)

if(NOT DEFINED WEBKIT_GLIB_API)
    set(SOURCE_LIST
        ${SOURCE_LIST}
        ORBInjectedBundle/Tags.cpp
        ORBInjectedBundle/Utils.cpp
)
endif()

add_library(${MODULE_NAME} SHARED
    ${SOURCE_LIST}
)

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=${MODULE_NAME})

target_link_libraries(${MODULE_NAME}
    PRIVATE
        CompileSettingsDebug::CompileSettingsDebug
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${WPE_BACKEND_LIBRARIES}
        ${LIBSOUP_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${WPE_WEBKIT_LIBRARIES}
        ${SECURITY_TOKEN_LIBRARY})

target_include_directories(${MODULE_NAME}
    PRIVATE
        ${WPE_BACKEND_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}/WPE
        ${GLIB_INCLUDE_DIRS}
        ${LIBSOUP_INCLUDE_DIRS}
)

if(WPE_WEBKIT_DEPRECATED_API)
    target_compile_definitions(${MODULE_NAME} PRIVATE WPE_WEBKIT_DEPRECATED_API=${WPE_WEBKIT_DEPRECATED_API})
endif()

if(WEBKIT_GLIB_API)
    target_compile_definitions(${MODULE_NAME} PRIVATE WEBKIT_GLIB_API=${WEBKIT_GLIB_API})
endif()

install(
    TARGETS ${MODULE_NAME}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${STORAGE_DIRECTORY}/plugins
    COMPONENT ${MODULE_NAME})

# also build injected bundle
add_subdirectory(ORBInjectedBundle)

# resolution handling
if(PLUGIN_ORBBROWSER_RESOLUTION EQUAL "720p")
    set(PLUGIN_ORBBROWSER_HEIGHT "720")
    set(PLUGIN_ORBBROWSER_WIDTH "1280")
elseif(PLUGIN_ORBBROWSER_RESOLUTION EQUAL "1080p")
    set(PLUGIN_ORBBROWSER_HEIGHT "1080")
    set(PLUGIN_ORBBROWSER_WIDTH "1920")
elseif(PLUGIN_ORBBROWSER_RESOLUTION EQUAL "2160p")
    set(PLUGIN_ORBBROWSER_HEIGHT "2160")
    set(PLUGIN_ORBBROWSER_WIDTH "3840")
elseif(PLUGIN_ORBBROWSER_RESOLUTION EQUAL "auto")
    set(PLUGIN_ORBBROWSER_HEIGHT "")
    set(PLUGIN_ORBBROWSER_WIDTH "")
endif()

write_config( ${PLUGIN_NAME} )
