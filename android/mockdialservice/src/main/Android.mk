LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DEBUG := true

LOCAL_PACKAGE_NAME := org.orbtv.mockdialservice

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/aidl

LOCAL_SRC_FILES := \
   $(call all-subdir-java-files) \
   $(call all-subdir-Iaidl-files)

LOCAL_JNI_SHARED_LIBRARIES := liborg.orbtv.mockdialservice.mockdialservice-jni

LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

ORB_COMPONENT_PATH := $(LOCAL_PATH)/../../../../components
#include $(call all-makefiles-under,$(LOCAL_PATH))
include $(LOCAL_PATH)/cpp/Android.mk
include $(ORB_COMPONENT_PATH)/mock_dial_service/Android.mk

