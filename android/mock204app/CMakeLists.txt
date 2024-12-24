cmake_minimum_required(VERSION 3.4.1)
project(Mock204App)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Use C++17 to fix error in binder header: "no member named 'optional' in namespace 'std'"
set(CMAKE_CXX_STANDARD 17)

option(USE_DEBUG "debug" OFF)
option(USE_ANDROID_DEBUG "android debug" OFF)
if (CMAKE_CURRENT_BINARY_DIR MATCHES "debug")
   add_definitions(-DMGR_DEBUG)
   add_definitions(-DANDROID_DEBUG)
endif()

add_definitions(
   -DCOMPILE_SDK_VERSION=31
)

include_directories(
   src/main/cpp/generated
   src/main/cpp/aidl/include/ndk
)

find_library(log-lib log)
find_library(android-lib android)
find_library(binder_ndk-lib binder_ndk)

add_library(
   org.orbtv.mock204app.native SHARED
   # sources...
   src/main/cpp/generated/aidl/org/orbtv/orbservice/IDvbiSession.cpp
   src/main/cpp/generated/aidl/org/orbtv/orbservice/IOrbcSession.cpp
   src/main/cpp/aidl/include/ndk/DataBuffer.cpp
   src/main/cpp/native.cpp
   src/main/cpp/jni_utils.cpp
   src/main/cpp/DvbiSession.cpp
)

target_link_libraries(
   org.orbtv.mock204app.native
   # libraries...
   ${binder_ndk-lib}
   ${android-lib}
   ${log-lib}
)
