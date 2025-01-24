LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
APP_DEBUG := true
LOCAL_MODULE := org.orbtv.orblibrary
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
else
    LOCAL_PRIVATE_PLATFORM_APIS := true
endif
ORB_AIDL_PATH := ../../../orbservice/src/main/cpp/aidl

LOCAL_SRC_FILES := $(call all-subdir-java-files) \
	$(ORB_AIDL_PATH)/org/orbtv/orbservice/IBridgeSession.aidl \
	$(ORB_AIDL_PATH)/org/orbtv/orbservice/IBrowserSession.aidl

LOCAL_AIDL_INCLUDES := $(LOCAL_PATH)/$(ORB_AIDL_PATH)
LOCAL_JNI_SHARED_LIBRARIES := liborg.orbtv.orblibrary.native
LOCAL_STATIC_JAVA_LIBRARIES += okio-1.17.2
LOCAL_STATIC_JAVA_LIBRARIES += okhttp-3.14.9
LOCAL_STATIC_JAVA_LIBRARIES += org.orbtv.orbpolyfill
include $(BUILD_STATIC_JAVA_LIBRARY)
##################################################
include $(CLEAR_VARS)
LOCAL_MODULE := okhttp-3.14.9
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
endif
LOCAL_SDK_VERSION := system_current
LOCAL_SRC_FILES := ../../../../external/okhttp/okhttp-3.14.9.jar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_PRODUCT_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
include $(BUILD_PREBUILT)
##################################################
include $(CLEAR_VARS)
LOCAL_MODULE := okio-1.17.2
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
endif
LOCAL_SDK_VERSION := system_current
LOCAL_SRC_FILES := ../../../../external/okhttp/okio-1.17.2.jar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_PRODUCT_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
include $(BUILD_PREBUILT)
##################################################
ORB_COMPONENT_PATH := $(LOCAL_PATH)/../../../../components
#include $(call all-makefiles-under,$(LOCAL_PATH))
include $(LOCAL_PATH)/cpp/Android.mk
include $(ORB_COMPONENT_PATH)/application_manager/Android.mk
include $(ORB_COMPONENT_PATH)/network_services/Android.mk

