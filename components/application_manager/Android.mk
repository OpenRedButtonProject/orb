LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.orblibrary.applicationmanager

LOCAL_SRC_FILES := \
   application_manager.cpp \
   utils.cpp \
   ait.cpp \
   xml_parser.cpp \
   app.cpp

LOCAL_CFLAGS := \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor

LOCAL_CPPFLAGS += -fexceptions

LOCAL_SHARED_LIBRARIES := \
   libxml2

include $(BUILD_STATIC_LIBRARY)
