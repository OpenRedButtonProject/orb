CPP_LOCAL_PATH := $(call my-dir)

LOCAL_PATH := $(CPP_LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.tvbrowser.applicationmanager-jni

LOCAL_SRC_FILES := \
   application_manager_native.cpp \
   jni_utils.cpp

LOCAL_CFLAGS := -Wno-unused-parameter

LOCAL_C_INCLUDES := \
   $(LOCAL_PATH)/../symlink.application_manager/

LOCAL_STATIC_LIBRARIES := \
   liborg.orbtv.tvbrowser.applicationmanager \
   liblog \
   libxml2

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH := $(CPP_LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.tvbrowser.networkservices-jni

LOCAL_SRC_FILES := \
   network_services_native.cpp \
   jni_utils.cpp

LOCAL_CFLAGS := -Wno-unused-parameter \
                -Wno-unused-variable \
                -Wno-unused-function \
                -Wno-reorder-ctor \
                -Wno-non-virtual-dtor \
                -Wno-unused-private-field

LOCAL_C_INCLUDES := \
   $(LOCAL_PATH)/../symlink.network_services/ \
   $(LOCAL_PATH)/../symlink.network_services/media_synchroniser/ \
   $(LOCAL_PATH)/../symlink.network_services/app2app/

LOCAL_SHARED_LIBRARIES := \
   libcap \
   libssl \
   libcrypto \
   libjsoncpp_ORB

LOCAL_STATIC_LIBRARIES := \
   libwebsockets \
   liborg.orbtv.tvbrowser.networkservices \
   liblog

include $(BUILD_SHARED_LIBRARY)

