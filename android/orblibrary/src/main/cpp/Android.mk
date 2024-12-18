CPP_LOCAL_PATH := $(call my-dir)

LOCAL_PATH := $(CPP_LOCAL_PATH)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.orblibrary.native

ifeq ($(ORB_HBBTV_VERSION),)
    $(error ORB_HBBTV_VERSION is not defined)
endif

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

LOCAL_SHARED_LIBRARIES += \
   libnativehelper

LOCAL_SRC_FILES := \
   jni_utils.cpp \
   application_manager_native.cpp \
   network_services_native.cpp \
   native.cpp

LOCAL_C_INCLUDES := \
   $(LOCAL_PATH)/../../../../../components/application_manager/ \
   $(LOCAL_PATH)/../../../../../components/network_services/ \
   $(LOCAL_PATH)/../../../../../components/network_services/media_synchroniser/ \
   $(LOCAL_PATH)/../../../../../components/network_services/app2app/

ifeq ($(ORB_HBBTV_VERSION),204)
LOCAL_SRC_FILES += json_rpc_native.cpp
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../components/network_services/json_rpc/
endif

LOCAL_STATIC_LIBRARIES += liborg.orbtv.orblibrary.applicationmanager
LOCAL_STATIC_LIBRARIES += liborg.orbtv.orblibrary.networkservices
LOCAL_STATIC_LIBRARIES += libxml2
LOCAL_STATIC_LIBRARIES += libwebsockets

LOCAL_CFLAGS := -Wno-unused-parameter \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field \
   -DORB_HBBTV_VERSION=$(ORB_HBBTV_VERSION) \
   -DBUILD_INFO='"$(LOCAL_MODULE) build $(shell date +"%Y-%m-%d %H:%M:%S")"'

include $(BUILD_SHARED_LIBRARY)
