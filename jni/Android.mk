LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

#########################################################################
#																		#
#								Includes								#
#																		#
#########################################################################
LOCAL_C_INCLUDES        :=  \
	$(JNI_H_INCLUDE)         \
	$(LOCAL_PATH)/include   \
	$(LOCAL_PATH)/../library/include \

#########################################################################
#																		#
#								Sources									#
#																		#
#########################################################################
LOCAL_SRC_FILES         :=  \
	nxrearcam_jni.cpp \
	nxcommand.cpp   \
	NX_DbgMsg.cpp   \

#########################################################################
#																		#
#								Build optios							#
#																		#
#########################################################################
LOCAL_CFLAGS := -O3 -DNDEBUG
LOCAL_CFLAGS += -Wall -Werror

LOCAL_CFLAGS += \
		-Wno-gnu-static-float-init \
		-Wno-non-literal-null-conversion \
		-Wno-self-assign \
		-Wno-unused-parameter \
		-Wno-unused-variable \
		-Wno-unused-function \

ANDROID_VERSION_STR := $(PLATFORM_VERSION)
ANDROID_VERSION := $(firstword $(ANDROID_VERSION_STR))
ifeq ($(ANDROID_VERSION), 9)
LOCAL_CFLAGS += -DANDROID_PIE
endif


#########################################################################
#																		#
#								Libs									#
#																		#
#########################################################################

LOCAL_SHARED_LIBRARIES += liblog libandroid libutils libhardware libdrm libgui

LOCAL_STATIC_LIBRARIES += libnxrearcam libnxdeinterlace libdeinterlacer_static libnx_v4l2 libnx_renderer

#########################################################################
#																		#
#								Target									#
#																		#
#########################################################################
LOCAL_MODULE            := libnxrearcam_jni

include $(BUILD_SHARED_LIBRARY)
