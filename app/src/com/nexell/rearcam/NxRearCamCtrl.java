package com.nexell.rearcam;

import android.view.Surface;

public class NxRearCamCtrl {
	private static final String NX_TAG = "NxRearCamCtrl";

	private Surface m_Surf = null;

	//public NxRearCamCtrl(int dspX, int dspY, int dspWidth, int dspHeight){
	public NxRearCamCtrl(){

	};

	public synchronized void RearCamSetParam(int screen_width, int screen_height)
	{
		NX_JniNxRearCamSetParam(screen_width, screen_height);
	}

	public synchronized int RearCamStart()
	{
		return NX_JniNxRearCamStart(m_Surf);
	}

	public synchronized boolean RearCamStop()
	{
		return NX_JniNxRearCamStop();
	}

	public synchronized int StartCmdService()
	{
		return NX_JniNxStartCmdService();
	}

	public synchronized int CheckReceivedStopCmd()
	{
		return NX_JniCheckReceivedStopCmd();
	}

	public synchronized int StopCmdService()
	{
		return NX_JniNXStopCommandService();
	}

	public synchronized void SetSurface(Surface surf)
	{
		m_Surf = surf;
	}

	public synchronized int GetDisplayWidth()
	{
		return NX_JniNXGetDisplayWidth();
	}

	public synchronized int GetDisplayHeight()
	{
		return NX_JniNXGetDisplayHeight();
	}

	public synchronized int GetDisplayPositionX()
	{
		return NX_JniNXGetDisplayPositionX();
	}

	public synchronized int GetDisplayPositionY()
	{
		return NX_JniNXGetDisplayPositionY();
	}

	static{
		System.loadLibrary("nxrearcam_jni");
	}

	public native int NX_JniNxRearCamSetParam(int lcdWidth, int lcdHeight);
	public native int NX_JniNxRearCamStart(Surface sf);
	public native boolean NX_JniNxRearCamStop();
	public native int NX_JniNxStartCmdService();
	public native int NX_JniCheckReceivedStopCmd();
	public native int NX_JniNXStopCommandService();
	public native int NX_JniNXGetDisplayWidth();
	public native int NX_JniNXGetDisplayHeight();
	public native int NX_JniNXGetDisplayPositionX();
	public native int NX_JniNXGetDisplayPositionY();

}
