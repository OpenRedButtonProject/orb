LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_DEBUG := true

LOCAL_PACKAGE_NAME := org.orbtv.voicerecognitionservice

LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := \
   $(call all-subdir-java-files)


LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_STATIC_JAVA_LIBRARIES += androidx-core
LOCAL_STATIC_JAVA_LIBRARIES += gson
LOCAL_STATIC_JAVA_LIBRARIES += aws-android-sdk-core
LOCAL_STATIC_JAVA_LIBRARIES += aws-android-sdk-mobile-client
LOCAL_STATIC_JAVA_LIBRARIES += aws-android-sdk-s3
LOCAL_STATIC_JAVA_LIBRARIES += aws-android-sdk-transcribe

include $(BUILD_PACKAGE)

include $(CLEAR_VARS)
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += gson:libs/gson-2.8.9.jar
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += androidx-core:libs/androidx-core-1.3.1.jar
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += aws-android-sdk-core:libs/aws-android-sdk-core-2.73.0.jar
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += aws-android-sdk-mobile-client:libs/aws-android-sdk-mobile-client-2.73.0.jar
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += aws-android-sdk-s3:libs/aws-android-sdk-s3-2.29.0.jar
LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES += aws-android-sdk-transcribe:libs/aws-android-sdk-transcribe-2.73.0.jar

include $(BUILD_MULTI_PREBUILT)
include $(call all-makefiles-under, $(LOCAL_PATH))

