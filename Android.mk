LOCAL_PATH:= $(call my-dir)

PREBUILT_LIBRARY_PATH := $(LOCAL_PATH)/library/prebuilt/
LIBRARY_PATH := $(LOCAL_PATH)/library
APP_PATH := $(LOCAL_PATH)/app
JNI_LIB_PATH := $(LOCAL_PATH)/jni

#include $(LIBRARY_PATH)/prebuilt/libnxdeinterlace/Android.mk
#include $(LIBRARY_PATH)/prebuilt/libtsdeinterlacer/Android.mk
include $(LIBRARY_PATH)/librearcam/Android.mk
include $(JNI_LIB_PATH)/Android.mk

include $(APP_PATH)/Android.mk