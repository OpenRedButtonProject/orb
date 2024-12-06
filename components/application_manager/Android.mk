LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.orblibrary.applicationmanager

ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
    LOCAL_SHARED_LIBRARIES := liblog
    LOCAL_STATIC_LIBRARIES := libxml2
else
    LOCAL_SHARED_LIBRARIES := libxml2
endif

LOCAL_SRC_FILES := \
   application_manager.cpp \
   utils.cpp \
   ait.cpp \
   xml_parser.cpp \
   hbbtv_app.cpp

LOCAL_CFLAGS := \
   -Wno-unused-variable \
   -Wno-unused-function \
   -DORB_HBBTV_VERSION=$(ORB_HBBTV_VERSION)

LOCAL_CPPFLAGS += -fexceptions

include $(BUILD_STATIC_LIBRARY)
