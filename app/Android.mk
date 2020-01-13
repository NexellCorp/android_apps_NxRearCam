LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

partition ?= system

LOCAL_PACKAGE_NAME := NxRearCam
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
#LOCAL_PRIVILEGED_MODULE := true
LOCAL_USE_AAPT2 := true

LOCAL_STATIC_JAVA_LIBRARIES += \
	android-support-v13 \


LOCAL_RESOURCE_DIR += \
	$(LOCAL_PATH)/res \


LOCAL_AAPT_FLAGS := --auto-add-overlay \


LOCAL_SRC_FILES += 	src/com/nexell/rearcam/MainActivity.java \
					src/com/nexell/rearcam/NxCmdReceiver.java \
					src/com/nexell/rearcam/NxRearCamCtrl.java \




LOCAL_SDK_VERSION := current

LOCAL_JNI_SHARED_LIBRARIES += libnxrearcam_jni

include $(BUILD_PACKAGE)

