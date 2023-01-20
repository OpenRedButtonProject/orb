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
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/iframe.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/hbbtv.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/dash.all.min.js"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/resources/assets/polyfill/playerpage.html"
)

add_custom_command(OUTPUT ${POLLYFILL_JS_FILES}
                   COMMAND make WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../../../
                   COMMENT "Creating polyfill objects")

add_custom_target(polyfill_js ALL DEPENDS ${POLLYFILL_JS_FILES})

install(DIRECTORY 
   DESTINATION ${JAVASCRIPT_PAYLOAD_PATH}
)

install(PROGRAMS ${POLLYFILL_JS_FILES} DESTINATION ${JAVASCRIPT_PAYLOAD_PATH})

