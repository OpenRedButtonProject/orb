##
# ORB Software. Copyright (c) 2022 Ocean Blue Software Limited
#
# Licensed under the ORB License that can be found in the LICENSE file at
# the top level of this repository.
#
set(PLUGIN_NAME ORBInjectedBundle)
set(MODULE_NAME WPE${PLUGIN_NAME})

find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(securityagent QUIET)
if(DEFINED WEBKIT_GLIB_API)
find_package(LibSoup REQUIRED)
endif()

option(PLUGIN_ORB "Enable ORB support." ON)

set(SOURCE_LIST
    main.cpp
    WhiteListedOriginDomainsList.cpp
)

if(NOT DEFINED WEBKIT_GLIB_API)
    set(SOURCE_LIST
        ${SOURCE_LIST}
        RequestHeaders.cpp
        Utils.cpp
        Tags.cpp
    )
endif()

add_library(${MODULE_NAME} SHARED
    ${SOURCE_LIST}
)

if(NOT DEFINED JAVASCRIPT_PAYLOAD_PATH)
    set(JAVASCRIPT_PAYLOAD_PATH ${CMAKE_INSTALL_PREFIX}/share/${NAMESPACE}/ORBBrowser)
endif()

if(PLUGIN_ORB)

    find_package(LibXml2)
    target_link_libraries(${MODULE_NAME} PRIVATE curl)

    include_directories(
        "."
        "orb"
    )

    target_sources(${MODULE_NAME} PRIVATE orb/ORBClient.cpp)
    target_sources(${MODULE_NAME} PRIVATE orb/WpeBridge.cpp)
    target_sources(${MODULE_NAME} PRIVATE orb/OrbUtils.cpp)
    target_compile_definitions(${MODULE_NAME} PRIVATE ENABLE_ORB)
    target_compile_definitions(${MODULE_NAME} PRIVATE JAVASCRIPT_PAYLOAD_PATH="${JAVASCRIPT_PAYLOAD_PATH}")
endif()

set_target_properties(${MODULE_NAME} PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED YES)

target_compile_definitions(${MODULE_NAME} PRIVATE MODULE_NAME=${MODULE_NAME})

target_link_libraries(${MODULE_NAME}
    PRIVATE 
       ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${LIBSOUP_LIBRARIES}
        ${GLIB_LIBRARIES}
        ${WPE_WEBKIT_LIBRARIES}
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        )

target_include_directories(${MODULE_NAME} 
    PRIVATE 
        ${WPE_WEBKIT_INCLUDE_DIRS}
        ${WPE_WEBKIT_INCLUDE_DIRS}/WPE
        ${LIBSOUP_INCLUDE_DIRS}
        ${GLIB_INCLUDE_DIRS})

if(WEBKIT_GLIB_API)
    target_compile_definitions(${MODULE_NAME} PRIVATE WEBKIT_GLIB_API=${WEBKIT_GLIB_API})
endif()

install(TARGETS ${MODULE_NAME} 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${NAMESPACE}/ORBBrowser
    COMPONENT ${MODULE_NAME})

set(POLLYFILL_JS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/hbbtv.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/dash.all.min.js"
)
  
add_custom_command(OUTPUT ${POLLYFILL_JS_FILES}
                    COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../
                    COMMENT "Creating polyfill objects")

add_custom_target(polyfill_js ALL DEPENDS ${POLLYFILL_JS_FILES})

install(PROGRAMS DESTINATION ${JAVASCRIPT_PAYLOAD_PATH})