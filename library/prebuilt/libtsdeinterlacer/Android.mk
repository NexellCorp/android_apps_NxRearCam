LOCAL_PATH		:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := libtsdeinterlacer
LOCAL_MODULE_CLASS := STATIC_LIBRARIES
LOCAL_MODULE_SUFFIX := .a
LOCAL_C_INCLUDE := $(LOCAL_PATH)/../../include
LOCAL_SRC_FILES := libdeinterlacer_static.a
include $(BUILD_PREBUILT)
