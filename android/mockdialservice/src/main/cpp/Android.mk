LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.mockdialservice.mockdialservice-jni

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_HEADER_LIBRARIES := jni_headers

LOCAL_SRC_FILES := \
   mock_dial_service_native.cpp \
   jni_utils.cpp

LOCAL_CFLAGS := -Wno-unused-parameter \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field

LOCAL_C_INCLUDES := \
   $(LOCAL_PATH)/../symlink.mock_dial_service/
   
LOCAL_STATIC_LIBRARIES := \
   liborg.dtvkit.inputsource.mockdialservice \
   liblog

include $(BUILD_SHARED_LIBRARY)
