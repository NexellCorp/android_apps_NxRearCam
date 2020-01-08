LOCAL_PATH		:= $(call my-dir)

include $(CLEAR_VARS)

#########################################################################
#																		#
#								Includes								#
#																		#
#########################################################################
LOCAL_C_INCLUDES        += \
	$(LOCAL_PATH)/../common \
	$(LOCAL_PATH)/../include \
	$(LOCAL_PATH)/../include/drm \

#########################################################################
#																		#
#								Sources									#
#																		#
#########################################################################
LOCAL_SRC_FILES			:=  \
	NX_CV4l2Camera.cpp \
	NX_CV4l2VipFilter.cpp \
	NX_CDeinterlaceFilter.cpp \
	NX_CVideoRenderFilter.cpp \
	NX_CAndroidRenderer.cpp \
	nx_video_alloc_ion.cpp \
	nx_video_alloc_drm.c \

#Utils
LOCAL_SRC_FILES			+= \
	../common/NX_DbgMsg.cpp \

# Manager
LOCAL_SRC_FILES			+= \
	NX_CRearCamManager.cpp \


#########################################################################
#																		#
#								Build optios							#
#																		#
#########################################################################
LOCAL_CFLAGS	+= -DTHUNDER_DEINTERLACE_TEST

ANDROID_VERSION_STR := $(PLATFORM_VERSION)
ANDROID_VERSION := $(firstword $(ANDROID_VERSION_STR))
ifeq ($(ANDROID_VERSION), 9)
LOCAL_CFLAGS	+= -DUI_OVERLAY_APP
else
LOCAL_CFLAGS 	+= -DPLATFORM_SDK_VERSION=$(PLATFORM_SDK_VERSION)
LOCAL_CFLAGS	+= -DUSE_ION_ALLOCATOR -DANDROID_SURF_RENDERING
endif



#########################################################################
#																		#
#								Target									#
#																		#
#########################################################################
LOCAL_MODULE		:= libnxrearcam

LOCAL_MODULE_TAGS	:= optional

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_STATIC_LIBRARY)
