package com.nexell.rearcam;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Point;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.Toast;

public class MainActivity extends Activity implements SurfaceHolder.Callback{
	private final String NX_DTAG = "MainActivity";

	public static Context m_Context;


	private int dspX;
	private int dspY;
	private int dspWidth;
	private int dspHeight;
	private int moduleIdx;

	private SurfaceView mSurfaceView;
	private SurfaceHolder mSurfaceHolder;
	private Surface mSurface;


	private Display display;
	private Point dspSize;

	private int ret;

	public NxRearCamCtrl m_RearCamCtrl;
	public NxCmdReceiver m_CmdReceiver;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		setContentView(R.layout.activity_main);

		m_Context = this;

		//----------------------------------------------------------------------------------
		//Display setting(full screen)
		getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
				| View.SYSTEM_UI_FLAG_FULLSCREEN
				| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
				| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
				| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
				| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
				| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
		);
		//----------------------------------------------------------------------------------

		mSurfaceView = (SurfaceView)findViewById(R.id.surfaceView);

		mSurfaceHolder = mSurfaceView.getHolder();

		mSurfaceHolder.addCallback(this);

		super.onCreate(savedInstanceState);

	}

	@Override
	protected void onResume() {

		//----------------------------------------------------------------------------------
		//Display setting(full screen)
		getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_LOW_PROFILE
				| View.SYSTEM_UI_FLAG_FULLSCREEN
				| View.SYSTEM_UI_FLAG_LAYOUT_STABLE
				| View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
				| View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
				| View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
				| View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
		);
		//----------------------------------------------------------------------------------

		super.onResume();
	}

	@Override
	protected void onPause() {
		m_RearCamCtrl.RearCamStop();
		m_RearCamCtrl.StopCmdService();
		finish();
		android.os.Process.killProcess(android.os.Process.myPid());
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}


	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.d(NX_DTAG, "-------surface created-------\n");

		mSurface = holder.getSurface();

		dspX = 0;
		dspY = 0;
		dspWidth = mSurfaceView.getWidth();
		dspHeight = mSurfaceView.getHeight();

		Log.d(NX_DTAG, "-------widht : "+dspWidth+"  height : "+dspHeight);

		m_RearCamCtrl = new NxRearCamCtrl(dspX, dspY, dspWidth, dspHeight);
		m_CmdReceiver = new NxCmdReceiver(m_Handler,m_RearCamCtrl);

		moduleIdx = m_RearCamCtrl.GetModuleIdx();

		Log.d(NX_DTAG, "-------module idx : "+moduleIdx);

		m_RearCamCtrl.SetModuleIdx(moduleIdx);

		m_RearCamCtrl.StartCmdService();
		m_CmdReceiver.setDaemon(true);
		m_CmdReceiver.start();

		m_RearCamCtrl.SetSurface(mSurface);

		m_RearCamCtrl.RearCamStart();
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		m_RearCamCtrl.RearCamStop();
		m_RearCamCtrl.StopCmdService();
		Log.d(NX_DTAG, "-------surface destroyed-------\n");
	}

	Handler m_Handler = new Handler(){
		@Override
		public void handleMessage(Message msg) {
			if(msg.arg1 == 1){
				m_CmdReceiver.interrupted();
				m_RearCamCtrl.StopCmdService();
				m_RearCamCtrl.RearCamStop();

				finish();
				android.os.Process.killProcess(android.os.Process.myPid());

			}
		}
	};

	public void onClickExit(View view)
	{
		m_RearCamCtrl.RearCamStop();
		m_RearCamCtrl.StopCmdService();
		finish();
		android.os.Process.killProcess(android.os.Process.myPid());
	}


}
