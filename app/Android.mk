LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

partition ?= system

LOCAL_PACKAGE_NAME := NxRearCam
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
#LOCAL_PRIVILEGED_MODULE := true
LOCAL_USE_AAPT2 := true

LOCAL_STATIC_JAVA_LIBRARIES := \
	android-support-v13 \
#	android-support-v7-appcompat \



# LOCAL_STATIC_JAVA_LIBRARIES := \
# 	android-support-v7-recyclerview \
#     android-support-v7-preference \
#     android-support-v7-appcompat \
#     android-support-v14-preference \
#     android-support-v17-preference-leanback \
#     android-support-v17-leanback \
# 	android-support-v13 \

LOCAL_RESOURCE_DIR += \
	$(LOCAL_PATH)/res \
#	frameworks/support/v7/appcompat/res \
	

LOCAL_AAPT_FLAGS := --auto-add-overlay \


LOCAL_SRC_FILES += 	src/com/nexell/rearcam/MainActivity.java \
					src/com/nexell/rearcam/NxCmdReceiver.java \
					src/com/nexell/rearcam/NxRearCamCtrl.java \




LOCAL_SDK_VERSION := current

LOCAL_JNI_SHARED_LIBRARIES += libnxrearcam_jni

include $(BUILD_PACKAGE)

