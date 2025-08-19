LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# Use jsoncpp_ORB or otherwise use default jsoncpp from source tree
USE_JSONCPP_ORB ?= 1

LOCAL_MODULE := liborg.orbtv.orblibrary.networkservices

ifeq ($(ORB_HBBTV_VERSION),)
    $(error ORB_HBBTV_VERSION is not defined)
endif

ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
    LOCAL_SHARED_LIBRARIES := liblog

    LOCAL_STATIC_LIBRARIES := \
        libcap \
        libssl \
        libwebsockets
else
    LOCAL_SHARED_LIBRARIES := \
        libcap \
        libssl

    LOCAL_STATIC_LIBRARIES := \
        libwebsockets
endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/media_synchroniser \
                    $(LOCAL_PATH)/app2app

ifeq ($(ORB_HBBTV_VERSION),204)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/json_rpc_server
endif

LOCAL_SRC_FILES := \
   app2app/app2app_local_service.cpp \
   app2app/app2app_remote_service.cpp \
   media_synchroniser/media_synchroniser.cpp \
   media_synchroniser/Correlation.cpp \
   media_synchroniser/CorrelatedClock.cpp \
   media_synchroniser/ClockBase.cpp \
   media_synchroniser/SysClock.cpp \
   media_synchroniser/ClockUtilities.cpp \
   service_manager.cpp \
   UdpSocketService.cpp \
   websocket_service.cpp \
   media_synchroniser/WallClockService.cpp \
   media_synchroniser/ContentIdentificationService.cpp \
   media_synchroniser/CSSUtilities.cpp \
   media_synchroniser/TimelineSyncService.cpp

ifeq ($(ORB_HBBTV_VERSION),204)
LOCAL_SRC_FILES += \
    json_rpc/JsonRpcService.cpp
endif

LOCAL_CFLAGS := \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-unused-parameter \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field \
   -DORB_HBBTV_VERSION=$(ORB_HBBTV_VERSION) \
   -Wno-reorder \
   -Wno-sign-compare \
   -Wno-pessimizing-move

ifeq ($(USE_JSONCPP_ORB), 1)
    LOCAL_SHARED_LIBRARIES += libjsoncpp_ORB
else
    LOCAL_STATIC_LIBRARIES += libjsoncpp
endif

LOCAL_CPPFLAGS += -fexceptions \
                  -frtti

LOCAL_SHARED_LIBRARIES += libcrypto

include $(BUILD_STATIC_LIBRARY)
