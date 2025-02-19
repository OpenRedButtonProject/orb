LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DEBUG := true

LOCAL_PACKAGE_NAME := org.orbtv.orbservice

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := \
   $(call all-subdir-java-files)

LOCAL_JNI_SHARED_LIBRARIES := liborg.orbtv.orbservice.native

LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

ORB_MODERATOR_PATH := $(LOCAL_PATH)/../../../../orblibrary
include $(LOCAL_PATH)/cpp/Android.mk
include $(ORB_MODERATOR_PATH)/Android.mk
