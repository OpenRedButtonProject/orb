LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.mock204app.native

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := \
   DataBuffer.cpp \
   native.cpp \
   jni_utils.cpp \
   DvbiService.cpp

LOCAL_CFLAGS := -Wno-unused-parameter \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/includes

LOCAL_STATIC_LIBRARIES := \
   liblog

include $(BUILD_SHARED_LIBRARY)
