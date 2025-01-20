LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.mock203app.native

LOCAL_PRIVATE_PLATFORM_APIS := true

ORB_AIDL_PATH := ../../../../orbservice/src/main/cpp/aidl

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/$(ORB_AIDL_PATH)

LOCAL_SRC_FILES := \
   $(ORB_AIDL_PATH)/org/orbtv/orbservice/IOrbcSession.aidl \
   $(ORB_AIDL_PATH)/org/orbtv/orbservice/IDvbiSession.aidl \
   native.cpp \
   jni_utils.cpp \
   DvbiSession.cpp

LOCAL_CFLAGS := -Wno-unused-parameter \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/includes

LOCAL_SHARED_LIBRARIES := \
   libbinder_ndk \
   libbinder \
   libbase \
   libutils \
   libandroid

LOCAL_STATIC_LIBRARIES := \
   liblog

include $(BUILD_SHARED_LIBRARY)
