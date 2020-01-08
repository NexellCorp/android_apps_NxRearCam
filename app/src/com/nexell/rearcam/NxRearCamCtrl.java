package com.nexell.rearcam;

import android.view.Surface;

public class NxRearCamCtrl {
	private static final String NX_TAG = "NxRearCamCtrl";

	private int m_dspX;
	private int m_dspY;
	private int m_dspWidth;
	private int m_dspHeight;
	private int m_moduleIdx;
	private Surface m_Surf = null;

	public NxRearCamCtrl(int dspX, int dspY, int dspWidth, int dspHeight){
		this.m_dspX = dspX;
		this.m_dspY = dspY;
		this.m_dspWidth = dspWidth;
		this.m_dspHeight = dspHeight;
	};

	public void SetModuleIdx(int moduleIdx)
	{
		m_moduleIdx = moduleIdx;
	}

	public synchronized int RearCamStart()
	{
		return NX_JniNxRearCamStart(m_dspX, m_dspY, m_dspWidth, m_dspHeight, m_moduleIdx, m_Surf);
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

	public synchronized int GetModuleIdx(){
		m_moduleIdx = NX_JniGetQuickRearCamModuleIdx();
		return m_moduleIdx;
	}

	static{
		System.loadLibrary("nxrearcam_jni");
	}

	public native int NX_JniNxRearCamStart(int dspX, int dspY, int dspWidth, int dspHeight, int moduleIdx, Surface sf);
	public native boolean NX_JniNxRearCamStop();
	public native int NX_JniNxStartCmdService();
	public native int NX_JniCheckReceivedStopCmd();
	public native int NX_JniNXStopCommandService();
	public native int NX_JniGetQuickRearCamModuleIdx();


}
