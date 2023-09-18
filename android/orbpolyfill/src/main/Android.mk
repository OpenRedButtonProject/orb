LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DEBUG := true

LOCAL_MODULE := org.orbtv.orbpolyfill

ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
else
    LOCAL_PRIVATE_PLATFORM_APIS := true
endif

LOCAL_SRC_FILES := $(call all-subdir-java-files)

include $(LOCAL_PATH)/../../../../BuildPolyfill.mk
IGNORED := $(call build-polyfill,$(LOCAL_PATH)/../../../..,$(PRODUCT_OUT)/org.orbtv.polyfill/resources/assets/polyfill,android)
ROOT_REL_PATH = $(shell echo "$(LOCAL_PATH)/" | sed -e 's/[a-zA-Z0-9._-]*\//..\//g')
LOCAL_JAVA_RESOURCE_DIRS := $(ROOT_REL_PATH)/$(PRODUCT_OUT)/org.orbtv.polyfill/resources

include $(BUILD_STATIC_JAVA_LIBRARY)
include $(call all-makefiles-under, $(LOCAL_PATH))

