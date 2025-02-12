LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.orbservice.native

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/aidl

LOCAL_SRC_FILES := \
   $(call all-subdir-Iaidl-files) \
   native.cpp \
   jni_utils.cpp \
   BridgeSession.cpp \
   BrowserCallback.cpp \
   DvbBrokerSession.cpp \
   DvbClientCallback.cpp

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../../../orblibrary/include

LOCAL_CFLAGS := -Wno-unused-parameter \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field

LOCAL_STATIC_LIBRARIES := \
   liblog \
   liborb

LOCAL_SHARED_LIBRARIES := \
   libbinder_ndk \
   libbinder \
   libbase \
   libutils \
   libandroid

include $(BUILD_SHARED_LIBRARY)
