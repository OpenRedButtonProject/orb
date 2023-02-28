LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DEBUG := true

LOCAL_MODULE := org.orbtv.orblibrary

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JNI_SHARED_LIBRARIES := liborg.orbtv.orblibrary.applicationmanager-jni \
                              liborg.orbtv.orblibrary.networkservices-jni

LOCAL_STATIC_JAVA_LIBRARIES := okhttp.com.android.art.release \
   org.orbtv.orbpolyfill

include $(BUILD_STATIC_JAVA_LIBRARY)
include $(call all-makefiles-under, $(LOCAL_PATH))

