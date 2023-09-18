CPP_LOCAL_PATH := $(call my-dir)

LOCAL_PATH := $(CPP_LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.orblibrary.native

ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
    
    LOCAL_SHARED_LIBRARIES := \
        libbase \
        libcutils \
        libutils \
        liblog
    
    LOCAL_STATIC_LIBRARIES := \
        libcap \
        libssl \
        libcrypto_static \
        libjsoncpp_ORB
        
    LOCAL_HEADER_LIBRARIES := jni_headers
else
    LOCAL_SHARED_LIBRARIES := \
        libcap \
        libssl \
        libcrypto \
        libjsoncpp_ORB
       
    LOCAL_STATIC_LIBRARIES := \
        liblog
endif

LOCAL_SRC_FILES := \
   jni_utils.cpp \
   application_manager_native.cpp \
   network_services_native.cpp \
   native.cpp

LOCAL_CFLAGS := -Wno-unused-parameter

LOCAL_C_INCLUDES := \
   $(LOCAL_PATH)/../symlink.application_manager/ \
   $(LOCAL_PATH)/../symlink.network_services/ \
   $(LOCAL_PATH)/../symlink.network_services/media_synchroniser/ \
   $(LOCAL_PATH)/../symlink.network_services/app2app/

LOCAL_STATIC_LIBRARIES += libxml2
LOCAL_STATIC_LIBRARIES += libwebsockets
LOCAL_STATIC_LIBRARIES += liborg.orbtv.orblibrary.applicationmanager
LOCAL_STATIC_LIBRARIES += liborg.orbtv.orblibrary.networkservices

LOCAL_CFLAGS := -Wno-unused-parameter \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field

include $(BUILD_SHARED_LIBRARY)
