LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.orbservice.native

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/aidl

LOCAL_SRC_FILES := \
   aidl/org/orbtv/orbservice/IDvbiSession.aidl \
   aidl/org/orbtv/orbservice/IOrbcSession.aidl \
   aidl/include/cpp/DataBuffer.cpp \
   native.cpp \
   jni_utils.cpp \
   OrbcSession.cpp

LOCAL_CFLAGS := -Wno-unused-parameter \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/aidl/include/cpp

LOCAL_STATIC_LIBRARIES := \
   liblog

LOCAL_SHARED_LIBRARIES := \
   libbinder_ndk \
   libbinder \
   libbase \
   libutils \
   libandroid

include $(BUILD_SHARED_LIBRARY)
