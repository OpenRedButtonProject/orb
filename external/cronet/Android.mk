LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := cronet-api
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
endif
LOCAL_SRC_FILES := cronet-api.jar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_PRODUCT_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := cronet-common
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
endif
LOCAL_SRC_FILES := cronet-common.jar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_PRODUCT_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := cronet-embedded
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
endif
LOCAL_SRC_FILES := cronet-embedded.jar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_PRODUCT_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := protobuf-javalite-3.22.3
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
endif
LOCAL_SRC_FILES := protobuf-javalite-3.22.3.jar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_SUFFIX := $(COMMON_JAVA_PACKAGE_SUFFIX)
LOCAL_PRODUCT_MODULE := true
LOCAL_UNINSTALLABLE_MODULE := true
include $(BUILD_PREBUILT)

# Java loads this as loadLibrary("cronet.119.0.6045.31") -> libcronet.119.0.6045.31.so.
# The module name is the installed filename, so it has to include the lib prefix.
include $(CLEAR_VARS)
LOCAL_MODULE := libcronet.119.0.6045.31
ifeq ($(ORB_VENDOR), true)
    LOCAL_VENDOR_MODULE := true
endif
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := both
LOCAL_MODULE_TARGET_ARCH := arm arm64 x86 x86_64
LOCAL_CHECK_ELF_FILES := false
LOCAL_SRC_FILES_arm := jni/armeabi-v7a/libcronet.119.0.6045.31.so
LOCAL_SRC_FILES_arm64 := jni/arm64-v8a/libcronet.119.0.6045.31.so
LOCAL_SRC_FILES_x86 := jni/x86/libcronet.119.0.6045.31.so
LOCAL_SRC_FILES_x86_64 := jni/x86_64/libcronet.119.0.6045.31.so
include $(BUILD_PREBUILT)
