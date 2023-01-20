LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DEBUG := true

LOCAL_MODULE := org.orbtv.tvbrowser

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JNI_SHARED_LIBRARIES := liborg.orbtv.tvbrowser.applicationmanager-jni \
                              liborg.orbtv.tvbrowser.networkservices-jni

LOCAL_STATIC_JAVA_LIBRARIES := okhttp.com.android.art.release

include $(LOCAL_PATH)/../../../../BuildPolyfill.mk
IGNORED := $(call build-polyfill,$(LOCAL_PATH)/../../../..,$(PRODUCT_OUT)/org.orbtv.polyfill/resources/assets/polyfill,android)
LOCAL_JAVA_RESOURCE_DIRS := ../../../../../../../$(PRODUCT_OUT)/org.orbtv.polyfill/resources

include $(BUILD_STATIC_JAVA_LIBRARY)
include $(call all-makefiles-under, $(LOCAL_PATH))

