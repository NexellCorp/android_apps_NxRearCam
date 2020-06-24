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


#ifdef ANDROID_PIE
	#define DEFAULT_MODULE			2
#else
	#define DEFAULT_MODULE			1
#endif

#define DEFAULT_CRTC_ID			26
#define DEFAULT_CONNECTOR_ID	41
#define DEFAULT_VIDEO_PLANE		27
#define DEFAULT_CAM_WIDTH		704
#define DEFAULT_CAM_HEIGHT		480
#define DEFAULT_INTERCAM_USE	1
#define DEFAULT_DEINTER_ENGINE	THUNDER_DEINTERLACER
#define DEFAULT_DEINTER_SENSE	3
#define DEFAULT_DP_X			420
#define DEFAULT_DP_Y			0
#define DEFAULT_DP_WIDTH		1080
#define DEFAULT_DP_HEIGHT		720
#define DEFAULT_LCD_WIDTH		1920
#define DEFAULT_LCD_HEIGHT		720
//
// APIs
//

typedef struct REARCAM_PARAM{
	int module;
	int connectorId;
	int crtcId;
	int videoPlaneId;
	int cam_width;
	int cam_height;
	int use_intercam;
	int deinter_engine;
	int deinter_corr;
	int pgl_en;
	int lcd_w;
	int lcd_h;
	int dp_w;
	int dp_h;
	int dp_x;
	int dp_y;
}_REARCAM_PARAM;


REARCAM_PARAM stRearCam_Param;

static ANativeWindow *window = NULL;
static void* pCmdHandle = NULL;
int crtc_id = 0;
int video_plane_id = 0;
int rgb_plane_id = 0;

const char *rearcam_module 				= "nx_cam.m=-m";
const char *rearcam_crtc_id 			= "nx_cam.c=-c";
const char *rearcam_video_pland_id 		= "nx_cam.v=-v";
const char *rearcam_cam_res 			= "nx_cam.r=-r";
const char *rearcam_display_pos 		= "nx_cam.D=-D";
const char *rearcam_deinter_engine 		= "nx_cam.d=-d";
const char *rearcam_deinter_sensitivity	= "nx_cam.s=-s";
const char *rearcam_dispaly_size 		= "nx_cam.R=-R";
const char *rearcam_lcd_res 			= "nx_cam.L=-L";
const char *rearcam_connector_id		= "nx_cam.n=-n";


static int GetSingleParameter(const char *param, int* arg)
{
	FILE *fp;
	int r_size = 0;
	char cmdline[4096];
	char *ptr;
	long crtc_id = 0;

	fp = fopen("/proc/cmdline", "r");
	if(fp == NULL)
	{
		NxDbgMsg(NX_DBG_INFO, "/proc/cmdline read failed \n");

		return -1;
	}

	r_size = fread(cmdline, 1, 4096, fp);
	if(r_size == 4096){
		NxDbgMsg(NX_DBG_INFO, "check size of cmdline!!\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);

	ptr = strstr(cmdline, param);
	if(ptr){
		int len = strlen(param);
		*arg = atoi(ptr+len);
	}else
	{
		return -1;
	}
	

	return 1;
}

static int GetComplexParameter(const char *param, int* arg1, int*arg2)
{
	FILE *fp;
	int r_size = 0;
	char cmdline[4096];
	char *ptr;

	fp = fopen("/proc/cmdline", "r");
	if(fp == NULL)
	{
		NxDbgMsg(NX_DBG_INFO, "/proc/cmdline read failed \n");

		return -1;
	}

	r_size = fread(cmdline, 1, 4096, fp);
	if(r_size == 4096){
		NxDbgMsg(NX_DBG_INFO, "check size of cmdline!!\n");
		fclose(fp);
		return -1;
	}

	fclose(fp);

	ptr = strstr(cmdline, param);

	if(ptr){
		int len = strlen(param);
		if(!strcmp(param, rearcam_display_pos))
		{
			sscanf(ptr+len, "%d,%d", arg1, arg2);
		}else
		{
			sscanf(ptr+len, "%dx%d", arg1, arg2);
		}
	}else
	{
		return -1;
	}
	

	return 1;
}

static void CalAspectRatio(int lcd_width, int lcd_height, int cam_width, int cam_height, int* dsp_width, int* dsp_height)
{
	int dp_w, dp_h;
	int margin_w, margin_h;

	margin_w = (lcd_width / 20); 
	margin_h = (lcd_height / 20);

	if(lcd_width > lcd_height)
	{
		dp_h = lcd_height;

		if(cam_width == 704)
		{
			dp_w = (720*dp_h)/cam_height;
		}else
		{
			dp_w = (cam_width*lcd_height)/cam_height;
		}
		if(dp_w > lcd_width || lcd_width-dp_w < margin_w)
			dp_w = lcd_width;
	}else
	{
		dp_w = lcd_width;

		if(cam_width == 704)
		{
			dp_h = (cam_height * dp_w)/720;
		}else
		{
			dp_h = (cam_height * dp_w)/cam_width;
		}
		if(dp_h > lcd_height || lcd_height-dp_h < margin_h)
			dp_h = lcd_height;
	}

	*dsp_width = dp_w;
	*dsp_height = dp_h;
}

static void CalDisplayPosition(int lcd_width, int lcd_height, int dsp_width, int dsp_height, int* dp_x, int* dp_y)
{
	*dp_x = (lcd_width - dsp_width) >> 1;
	*dp_y = (lcd_height - dsp_height) >> 1;
}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNxRearCamSetParam( JNIEnv *env, jclass obj, jint lcd_width, jint lcd_height)
{
	int32_t iRet = -1;

	stRearCam_Param.use_intercam 	= 1;
	stRearCam_Param.pgl_en 			= 0;

#ifndef GET_QUICKREARCAM_PARAM
	stRearCam_Param.module 		= DEFAULT_MODULE;
	stRearCam_Param.connectorId 	= DEFAULT_CONNECTOR_ID;
	stRearCam_Param.crtcId			= DEFAULT_CRTC_ID;
	stRearCam_Param.videoPlaneId	= DEFAULT_VIDEO_PLANE;
	stRearCam_Param.cam_width		= DEFAULT_CAM_WIDTH;
	stRearCam_Param.cam_height		= DEFAULT_CAM_HEIGHT;
	stRearCam_Param.deinter_engine	= DEFAULT_DEINTER_ENGINE;
	stRearCam_Param.deinter_corr	= DEFAULT_DEINTER_SENSE;
	stRearCam_Param.lcd_w			= lcd_width;
	stRearCam_Param.lcd_h			= lcd_height;

#ifndef FULLSCREEN_DSIPLAY
	stRearCam_Param.dp_w			= DEFAULT_DP_WIDTH;
	stRearCam_Param.dp_h			= DEFAULT_DP_HEIGHT;
	stRearCam_Param.dp_x			= DEFAULT_DP_X;
	stRearCam_Param.dp_y			= DEFAULT_DP_Y;
	//CalAspectRatio(lcd_width, lcd_height, 	stRearCam_Param.cam_width, stRearCam_Param.cam_height, &stRearCam_Param.dp_w, &stRearCam_Param.dp_h);
	//CalDisplayPosition(lcd_width, lcd_height, stRearCam_Param.dp_w, stRearCam_Param.dp_h, &stRearCam_Param.dp_x, &stRearCam_Param.dp_y);
#else
	stRearCam_Param.dp_w 	= lcd_width;
	stRearCam_Param.dp_h	= lcd_height;
	stRearCam.Param.dp_x	= 0;
	stRearCam.Param.dp_y	= 0;
#endif
#else
	
	iRet = GetSingleParameter(rearcam_module, &stRearCam_Param.module);
	if(iRet <0) stRearCam_Param.module = DEFAULT_MODULE;

	iRet = GetSingleParameter(rearcam_connector_id, &stRearCam_Param.connectorId);
	if(iRet <0) stRearCam_Param.connectorId = DEFAULT_CONNECTOR_ID;

	iRet = GetSingleParameter(rearcam_crtc_id, &stRearCam_Param.crtcId);
	if(iRet < 0) stRearCam_Param.crtcId = DEFAULT_CRTC_ID;

	iRet = GetSingleParameter(rearcam_video_pland_id, &stRearCam_Param.videoPlaneId);
	if(iRet <0) stRearCam_Param.videoPlaneId = DEFAULT_VIDEO_PLANE;

	iRet = GetComplexParameter(rearcam_cam_res, &stRearCam_Param.cam_width, &stRearCam_Param.cam_height);
	if(iRet <0) 
	{
		stRearCam_Param.cam_width = DEFAULT_CAM_WIDTH;
		stRearCam_Param.cam_height = DEFAULT_CAM_HEIGHT;
	}

	iRet = GetSingleParameter(rearcam_deinter_engine, &stRearCam_Param.deinter_engine);
	if(iRet <0) stRearCam_Param.deinter_engine = DEFAULT_DEINTER_ENGINE;

	iRet = GetSingleParameter(rearcam_deinter_sensitivity, &stRearCam_Param.deinter_corr);
	if(iRet <0) stRearCam_Param.deinter_corr = DEFAULT_DEINTER_SENSE;

	iRet = GetComplexParameter(rearcam_display_pos, &stRearCam_Param.dp_x, &stRearCam_Param.dp_y);
	if(iRet <0) 
	{
		stRearCam_Param.dp_x = DEFAULT_DP_X;
		stRearCam_Param.dp_y = DEFAULT_DP_Y;
	}

	iRet = GetComplexParameter(rearcam_dispaly_size, &stRearCam_Param.dp_w, &stRearCam_Param.dp_h);
	if(iRet <0) 
	{
		stRearCam_Param.dp_w = DEFAULT_DP_WIDTH;
		stRearCam_Param.dp_h = DEFAULT_DP_HEIGHT;
	}

	iRet = GetComplexParameter(rearcam_lcd_res, &stRearCam_Param.lcd_w, &stRearCam_Param.lcd_h);
	if(iRet <0) 
	{
		stRearCam_Param.lcd_w = DEFAULT_LCD_WIDTH;
		stRearCam_Param.lcd_h = DEFAULT_LCD_HEIGHT;
	}

#endif

	if(stRearCam_Param.module == 6 || stRearCam_Param.module == 9)  //TP2825
	{
		stRearCam_Param.cam_width = 1280;
		stRearCam_Param.cam_height = 720;
		stRearCam_Param.deinter_engine = NON_DEINTERLACER;
	}


	return 1;

}


//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNxRearCamStart( JNIEnv *env, jclass obj, jobject sf)
{

	NxDbgMsg( NX_DBG_VBS, "%s()++", __FUNCTION__ );

	int32_t iRet = -1;

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

	vip_info.iModule 		= stRearCam_Param.module;

	vip_info.iSensor		= nx_sensor_subdev;
	vip_info.iClipper		= nx_clipper_subdev;
	vip_info.bUseMipi		= false;
	vip_info.bUseInterCam   = stRearCam_Param.use_intercam;

	vip_info.iWidth			= stRearCam_Param.cam_width;
	vip_info.iHeight		= stRearCam_Param.cam_height;

	vip_info.iCropX			= 0;
	vip_info.iCropY 		= 0;
	vip_info.iCropWidth		= vip_info.iWidth;
	vip_info.iCropHeight	= vip_info.iHeight;
	vip_info.iOutWidth 		= vip_info.iWidth;
	vip_info.iOutHeight 	= vip_info.iHeight;

//--------deinterlacer info setting(for deinterlace engine)---------
	dsp_info.iConnectorId   = stRearCam_Param.connectorId;
	dsp_info.iPlaneId		= stRearCam_Param.videoPlaneId;
	dsp_info.iCrtcId		= stRearCam_Param.crtcId;
	dsp_info.uDrmFormat		= DRM_FORMAT_YUV420;
	dsp_info.iSrcWidth		= vip_info.iWidth;
	dsp_info.iSrcHeight		= vip_info.iHeight;
	dsp_info.iCropX			= 0;
	dsp_info.iCropY			= 0;
	dsp_info.iCropWidth		= vip_info.iWidth;
	dsp_info.iCropHeight	= vip_info.iHeight;
	dsp_info.iDspX			= stRearCam_Param.dp_x;
	dsp_info.iDspY			= stRearCam_Param.dp_y;
	dsp_info.iDspWidth 		= stRearCam_Param.dp_w;
	dsp_info.iDspHeight		= stRearCam_Param.dp_h;
	dsp_info.iPglEn         = stRearCam_Param.pgl_en;
	dsp_info.iLCDWidth      = stRearCam_Param.lcd_w;
	dsp_info.iLCDHeight		= stRearCam_Param.lcd_h;
	dsp_info.bSetCrtc		= 0;

//--------deinterlacer info setting(for deinterlace engine)---------
	deinter_info.iWidth		= stRearCam_Param.cam_width;
	deinter_info.iHeight	= stRearCam_Param.cam_height;
	deinter_info.iCorr		= stRearCam_Param.deinter_corr;


	if(vip_info.bUseInterCam == false)
	{
		deinter_info.iEngineSel = NON_DEINTERLACER;
	}else
	{
		deinter_info.iEngineSel = stRearCam_Param.deinter_engine;
	}

	if(sf != NULL)
	{
		window = ANativeWindow_fromSurface( env, sf );
		dsp_info.m_pNativeWindow = (void*)window;
	}else
	{
		dsp_info.m_pNativeWindow = NULL;
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
JNIEXPORT jint JNICALL NX_JniNXGetDisplayWidth( JNIEnv *env, jclass obj )
{
	return stRearCam_Param.dp_w;
}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNXGetDisplayHeight( JNIEnv *env, jclass obj )
{
	return stRearCam_Param.dp_h;
}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNXGetDisplayPositionX( JNIEnv *env, jclass obj )
{
	return stRearCam_Param.dp_x;
}

//------------------------------------------------------------------------------
JNIEXPORT jint JNICALL NX_JniNXGetDisplayPositionY( JNIEnv *env, jclass obj )
{
	return stRearCam_Param.dp_y;
}


//
// JNI Initialize
//

//------------------------------------------------------------------------------
static JNINativeMethod sMethods[] = {
		//	Native Function Name,       Sigunature,                     C++ Function Name
		{ "NX_JniNxRearCamSetParam",		        "(II)I",  (void*)NX_JniNxRearCamSetParam 	},
		{ "NX_JniNxRearCamStart",		        "(Landroid/view/Surface;)I",    (void*)NX_JniNxRearCamStart 	},
		{ "NX_JniNxRearCamStop",		        "()Z", 	                        (void*)NX_JniNxRearCamStop 	},
		{ "NX_JniNxStartCmdService",		    "()I", 	                        (void*)NX_JniNxStartCmdService 	},
		{ "NX_JniCheckReceivedStopCmd",		    "()I", 	                        (void*)NX_JniCheckReceivedStopCmd 	},
		{ "NX_JniNXStopCommandService",		    "()I", 	                        (void*)NX_JniNXStopCommandService 	},
		{ "NX_JniNXGetDisplayWidth",		    "()I", 	                        (void*)NX_JniNXGetDisplayWidth 	},
		{ "NX_JniNXGetDisplayHeight",		    "()I", 	                        (void*)NX_JniNXGetDisplayHeight 	},
		{ "NX_JniNXGetDisplayPositionX",		    "()I", 	                    (void*)NX_JniNXGetDisplayPositionX 	},
		{ "NX_JniNXGetDisplayPositionY",		    "()I", 	                    (void*)NX_JniNXGetDisplayPositionY 	},
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