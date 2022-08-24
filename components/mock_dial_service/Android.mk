LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.dtvkit.inputsource.mockdialservice

LOCAL_SRC_FILES := \
        url_lib.c \
        dial_data.c \
        dial_server.c \
        quick_ssdp.c \
        mongoose.c
   
LOCAL_CFLAGS := \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-unused-parameter \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field

LOCAL_CPPFLAGS += -fexceptions

include $(BUILD_STATIC_LIBRARY)