LOCAL_PATH := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
include $(LOCAL_PATH)/BuildPolyfill.mk
build: .FORCE
	$(call build-polyfill,$(LOCAL_PATH),$(LOCAL_PATH)/out/resources/assets/polyfill,$(ORB_NATIVE),$(BBC_API_ENABLE))
.FORCE:

