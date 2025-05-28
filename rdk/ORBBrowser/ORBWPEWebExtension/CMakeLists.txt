cmake_minimum_required (VERSION 2.8.11)

#project(ORBWPEWebExtension CXX)

set(TARGET_NAME ORBWPEWebExtension)

set(SOURCE_FILES
   src/ORBWPEWebExtension.cpp
)

# where to install the artifacts
set(INSTALL_DIR
   ${CMAKE_INSTALL_PREFIX}/lib/orb
)

add_library(${TARGET_NAME} SHARED
   ${SOURCE_FILES}
)

set_target_properties(${TARGET_NAME} PROPERTIES
   CXX_STANDARD 14
   CXX_STANDARD_REQUIRED YES
   LINKER_LANGUAGE CXX
)

# where to look for header files during build-time
target_include_directories(${TARGET_NAME} PRIVATE
   ${WPE_WEBKIT_INCLUDE_DIRS}
   ${GLIB_INCLUDE_DIRS}
   ${LIBSOUP_INCLUDE_DIRS}
   ../../ORB/library/src/core
   ../../ORB/library/src/core/api
   ../../ORB/library/src/core/utilities
)

target_link_libraries(${TARGET_NAME} PRIVATE
   ORBClient
)

# create the installation home dir if needed
install(DIRECTORY 
   DESTINATION ${INSTALL_DIR}
)

# install the shared library
install(TARGETS ${TARGET_NAME}
   DESTINATION "${INSTALL_DIR}"
)

if(NOT DEFINED JAVASCRIPT_PAYLOAD_PATH)
   set(JAVASCRIPT_PAYLOAD_PATH ${CMAKE_INSTALL_PREFIX}/share/${NAMESPACE}/ORBBrowser)
endif()

set(POLLYFILL_JS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/hbbtv.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/dash.all.min.js"
)


set(BBC_API_ENABLE_VALUE 0)
if(DEFINED BBC_API_ENABLE)
   set(BBC_API_ENABLE_VALUE ${BBC_API_ENABLE})
endif()

# provide ORB_HBBTV_VERSION through recipe for 204
set(ORB_HBBTV_VERSION_VALUE 203)
if (DEFINED ORB_HBBTV_VERSION)
   set(ORB_HBBTV_VERSION_VALUE ${ORB_HBBTV_VERSION})
endif()
add_definitions(-DORB_HBBTV_VERSION=${ORB_HBBTV_VERSION_VALUE})

add_custom_command(OUTPUT ${POLLYFILL_JS_FILES}
                   COMMAND ORB_NATIVE=rdk BBC_API_ENABLE=${BBC_API_ENABLE_VALUE} ORB_HBBTV_VERSION=${ORB_HBBTV_VERSION_VALUE} make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../
                   COMMENT "Creating polyfill objects")
add_custom_target(polyfill_js ALL DEPENDS ${POLLYFILL_JS_FILES})

install(DIRECTORY 
   DESTINATION ${JAVASCRIPT_PAYLOAD_PATH}
)

install(PROGRAMS ${POLLYFILL_JS_FILES} DESTINATION ${JAVASCRIPT_PAYLOAD_PATH})

