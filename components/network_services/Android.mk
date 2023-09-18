LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborg.orbtv.orblibrary.networkservices

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

LOCAL_CFLAGS := \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-reorder-ctor \
   -Wno-unused-parameter \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field
   

LOCAL_CPPFLAGS += -fexceptions \
                  -frtti

LOCAL_SHARED_LIBRARIES += libcrypto
LOCAL_SHARED_LIBRARIES += libjsoncpp_ORB

include $(BUILD_STATIC_LIBRARY)
