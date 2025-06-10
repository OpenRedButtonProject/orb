LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := liborb

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include

LOCAL_SRC_FILES := \
	moderator/AppManager.cpp \
	moderator/Network.cpp \
	moderator/MediaSynchroniser.cpp \
	moderator/Moderator.cpp

LOCAL_CFLAGS := \
   -DORB_HBBTV_VERSION=$(ORB_HBBTV_VERSION) \
   -Wno-unused-variable \
   -Wno-unused-function \
   -Wno-unused-parameter \
   -Wno-non-virtual-dtor \
   -Wno-unused-private-field \
   -Wno-reorder \
   -Wno-sign-compare \
   -Wno-pessimizing-move

LOCAL_SHARED_LIBRARIES := \
   libbase \
   libandroid \
   libjsoncpp

#LOCAL_CPPFLAGS += -fexceptions \
#                  -frtti

include $(BUILD_STATIC_LIBRARY)
