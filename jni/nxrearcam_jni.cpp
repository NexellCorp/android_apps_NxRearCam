//
// Created by jmbahn on 18. 10. 15.
//



#include <android/log.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <android/native_window.h>
#include <android/native_window_jni.h>


#include <NX_RearCam.h>
#include <nx-v4l2.h>
#include <drm_fourcc.h>

#include <nxcommand.h>

#define NX_DTAG     "libnxrearcam"

#define COMMAND_FILE_NAME "/data/data/com.nexell.backgearservice/rearcam_cmd"

//
// Debug tools
//
#define NX_DBG_VBS			2	// ANDROID_LOG_VERBOSE
#define NX_DBG_DEBUG		3	// ANDROID_LOG_DEBUG
#define	NX_DBG_INFO			4	// ANDROID_LOG_INFO
#define	NX_DBG_WARN			5	// ANDROID_LOG_WARN
#define	NX_DBG_ERR			6	// ANDROID_LOG_ERROR
#define NX_DBG_DISABLE		9

enum {
	REAR_CAM_STATUS_STOP = 0,
	REAR_CAM_STATUS_INIT ,
	REAR_CAM_STATUS_RUNNING,
	RAER_CAM_STATUS_MAX
};

int gNxRearCamDebugLevel 	= NX_DBG_ERR;

#define DBG_PRINT			__android_log_print
#define NxTrace(...)		DBG_PRINT(ANDROID_LOG_VERBOSE, NX_DTAG, __VA_ARGS__);

#define NxDbgMsg(A, ...)	do {										\
								if( gNxRearCamDebugLevel <= A ) {		\
									DBG_PRINT(A, NX_DTAG, __VA_ARGS__);	\
								}										\
							} while(0)

//
// APIs
//
static ANativeWindow *window = NULL;
static void* pCmdHandle = NULL;

const char *rearcam_module = "nx_cam.m=-m";

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNxRearCamStart( JNIEnv *env, jclass obj, jint dspX, jint dspY, jint dspWidth, jint dspHeight, jint moduleIdx, jobject sf)
{

	NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

	int32_t iRet = -1;

	//int32_t iModule = 2;
	int32_t iModule = moduleIdx;
	uint32_t connectorId = 41;
	int32_t crtcId = 26;
	int32_t videoPlaneId = 27;
	int32_t cam_width = 704;
	int32_t cam_height = 480;
	int32_t use_intercam = 1;
	//int32_t deinter_engine = NEXELL_DEINTERLACER;
	int32_t deinter_engine = THUNDER_DEINTERLACER;
	//int32_t deinter_engine = NON_DEINTERLACER;
	int32_t deinter_corr = 3;
#ifdef ANDROID_PIE
	int32_t dp_x = 420;
	int32_t dp_y = 0;
#else
	int32_t dp_x = (int32_t)dspX;
	int32_t dp_y = (int32_t)dspY;
#endif
	int32_t dp_w = (int32_t)dspWidth;;
	int32_t dp_h = (int32_t)dspHeight;
	int32_t pgl_en = 0;
	int32_t lcd_w = 1920;
	int32_t lcd_h = 720;

	NX_REARCAM_INFO vip_info;
	DISPLAY_INFO dsp_info;
	DEINTERLACE_INFO deinter_info;

	iRet = NX_RearCamGetStatus();

	if(iRet != REAR_CAM_STATUS_STOP)
	{
		return 0;
	}


	memset( &vip_info, 0x00, sizeof(vip_info) );
	memset( &dsp_info, 0x00, sizeof(dsp_info) );

//-------------------camera info setting---------------------------
	vip_info.iType			= CAM_TYPE_VIP;

	vip_info.iModule 		= iModule;

	vip_info.iSensor		= nx_sensor_subdev;
	vip_info.iClipper		= nx_clipper_subdev;
	vip_info.bUseMipi		= false;
	vip_info.bUseInterCam   = use_intercam;

	vip_info.iWidth			= cam_width;
	vip_info.iHeight		= cam_height;

	vip_info.iCropX			= 0;
	vip_info.iCropY 		= 0;
	vip_info.iCropWidth		= vip_info.iWidth;
	vip_info.iCropHeight	= vip_info.iHeight;
	vip_info.iOutWidth 		= vip_info.iWidth;
	vip_info.iOutHeight 	= vip_info.iHeight;

//--------deinterlacer info setting(for deinterlace engine)---------
	dsp_info.iConnectorId   = connectorId;
	dsp_info.iPlaneId		= videoPlaneId;
	dsp_info.iCrtcId		= crtcId;
	dsp_info.uDrmFormat		= DRM_FORMAT_YUV420;
	dsp_info.iSrcWidth		= vip_info.iWidth;
	dsp_info.iSrcHeight		= vip_info.iHeight;
	dsp_info.iCropX			= 0;
	dsp_info.iCropY			= 0;
	dsp_info.iCropWidth		= vip_info.iWidth;
	dsp_info.iCropHeight	= vip_info.iHeight;
	dsp_info.iDspX			= dp_x;
	dsp_info.iDspY			= dp_y;
	dsp_info.iDspWidth 		= dp_w;
	dsp_info.iDspHeight		= dp_h;
	dsp_info.iPglEn         = pgl_en;
	dsp_info.iLCDWidth      = lcd_w;
	dsp_info.bSetCrtc		= lcd_h;

//--------deinterlacer info setting(for deinterlace engine)---------
	deinter_info.iWidth		= cam_width;
	deinter_info.iWidth		= cam_width;
	deinter_info.iHeight	= cam_height;
	deinter_info.iCorr       = deinter_corr;


	if(sf != NULL)
	{
		window = ANativeWindow_fromSurface( env, sf );

		dsp_info.m_pNativeWindow = (void*)window;
	}else
	{
		dsp_info.m_pNativeWindow = NULL;
	}



	deinter_info.iWidth		= cam_width;
	deinter_info.iHeight	= cam_height;


	if(vip_info.bUseInterCam == false)
	{
		deinter_info.iEngineSel = NON_DEINTERLACER;
	}else
	{
		deinter_info.iEngineSel = deinter_engine;
	}

	NX_RearCamInit( &vip_info, &dsp_info, &deinter_info);
	NX_RearCamStart();


	NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
	return 1;

}

//------------------------------------------------------------------------------
JNIEXPORT jboolean JNICALL NX_JniNxRearCamStop( JNIEnv *env, jclass obj )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

	int iRet = -1;

	iRet = NX_RearCamGetStatus();

	if(iRet == REAR_CAM_STATUS_STOP)
		return 0;

	NX_RearCamSetDisplayVideoPriority(2);

	NX_RearCamDeInit();

	if(window != NULL)
	{
		ANativeWindow_acquire(window);
		ANativeWindow_release(window);
		window = NULL;
	}

	NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
	return 1;

}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNxStartCmdService( JNIEnv *env, jclass obj )
{
	if(pCmdHandle == NULL)
	{
		pCmdHandle = NX_GetCommandHandle();
	}

	NX_StartCommandService(pCmdHandle, (char *)COMMAND_FILE_NAME);

	return 0;
}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniCheckReceivedStopCmd( JNIEnv *env, jclass obj )
{
	int ret = 0;
	if(pCmdHandle != NULL)
	{
		ret = NX_CheckReceivedStopCmd(pCmdHandle);
	}

	return ret;
}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNXStopCommandService( JNIEnv *env, jclass obj )
{
	if(pCmdHandle != NULL)
	{
		NX_StopCommandService(pCmdHandle, (char *)COMMAND_FILE_NAME);
	}
	return 0;
}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniGetQuickRearCamModuleIdx( JNIEnv *env, jclass obj)
{
	FILE *fp;
	int r_size = 0;
	char cmdline[2048];
	char *ptr;
	long moduleIdx = 0;

	fp = fopen("/proc/cmdline", "r");
	if(fp == NULL)
	{
		NxDbgMsg(NX_DBG_INFO, "/proc/cmdline read failed \n");

#ifdef ANDROID_PIE
		return 2;
#else
		return 1;
#endif
	}

	r_size = fread(cmdline, 1, 2048, fp);
	if(r_size == 2048){
		NxDbgMsg(NX_DBG_INFO, "check size of cmdline!!\n");

#ifdef ANDROID_PIE
		return 2;
#else
		return 1;
#endif
	}

	fclose(fp);

	ptr = strstr(cmdline, rearcam_module);
	if(ptr){
		int len = strlen(rearcam_module);
		moduleIdx = strtol(ptr+len, NULL, 10);
	}else
	{
#ifdef ANDROID_PIE
		moduleIdx = 2;
#else
		moduleIdx = 1;
#endif
	}

	return (int)moduleIdx;
}


//
// JNI Initialize
//

//------------------------------------------------------------------------------
static JNINativeMethod sMethods[] = {
		//	Native Function Name,       Sigunature,                     C++ Function Name
		{ "NX_JniNxRearCamStart",		        "(IIIIILandroid/view/Surface;)I",(void*)NX_JniNxRearCamStart 	},
		{ "NX_JniNxRearCamStop",		        "()Z", 	                        (void*)NX_JniNxRearCamStop 	},
		{ "NX_JniNxStartCmdService",		    "()I", 	                        (void*)NX_JniNxStartCmdService 	},
		{ "NX_JniCheckReceivedStopCmd",		    "()I", 	                        (void*)NX_JniCheckReceivedStopCmd 	},
		{ "NX_JniNXStopCommandService",		    "()I", 	                        (void*)NX_JniNXStopCommandService 	},
		{ "NX_JniGetQuickRearCamModuleIdx",		    "()I", 	                    (void*)NX_JniGetQuickRearCamModuleIdx 	},
};

//------------------------------------------------------------------------------
static int RegisterNativeMethods( JNIEnv *env, const char *className, JNINativeMethod *gMethods, int numMethods )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

	jclass clazz;
	int result = JNI_FALSE;

	clazz = env->FindClass( className );
	if( clazz == NULL ) {
		NxTrace( "%s(): Native registration unable to find class '%s'", __FUNCTION__, className );
		goto FAIL;
	}

	if( env->RegisterNatives( clazz, gMethods, numMethods) < 0 ) {
		NxTrace( "%s(): RegisterNatives failed for '%s'", __FUNCTION__, className);
		goto FAIL;
	}

	result = JNI_TRUE;

FAIL:
	NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
	return result;
}

//------------------------------------------------------------------------------
jint JNI_OnLoad( JavaVM *vm, void *reserved )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

	jint result = -1;
	JNIEnv *env = NULL;

	if( vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK ) {
		NxTrace( "%s(): GetEnv failed!\n", __FUNCTION__ );
		goto FAIL;
	}

	if( RegisterNativeMethods(env, "com/nexell/rearcam/NxRearCamCtrl", sMethods, sizeof(sMethods) / sizeof(sMethods[0]) ) != JNI_TRUE ) {
		NxTrace( "%s(): RegisterNativeMethods failed!", __FUNCTION__ );
		goto FAIL;
	}

	result = JNI_VERSION_1_4;

FAIL:
	NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
	return result;
}

//------------------------------------------------------------------------------
void JNI_OnUnload( JavaVM *vm, void *reserved )
{
	NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

	JNIEnv *env = NULL;

	if( vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK ) {
		NxTrace( "%s(): GetEnv failed!", __FUNCTION__ );
		goto FAIL;
	}

FAIL:
	NxDbgMsg( NX_DBG_VBS, "%s()--", __FUNCTION__ );
}

//-----------------------------------------------------------------------------