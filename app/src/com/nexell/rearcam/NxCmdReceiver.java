package com.nexell.rearcam;

import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.File;
import java.io.RandomAccessFile;

public class NxCmdReceiver extends Thread{
	private final String NX_DTAG = "NxCmdReceiver";

	Handler m_Handler;
	NxRearCamCtrl m_CamCtrl;

	private int received_stop_cmd = 0;
	private boolean m_ExitLoop = false;

	NxCmdReceiver(Handler handler, NxRearCamCtrl camctrl)
	{
		this.m_Handler = handler;
		this.m_CamCtrl = camctrl;
	}

	@Override
	public void run() {
		Message msg = new Message();

		received_stop_cmd = 0;

		while (!m_ExitLoop) {
			received_stop_cmd = m_CamCtrl.CheckReceivedStopCmd();

			//Log.d(NX_DTAG, "==================received_stop_cmd : " + received_stop_cmd);
			if (received_stop_cmd == 1) {
				msg.what = 0;
				msg.arg1 = 1;
				m_Handler.sendMessage(msg);

				m_ExitLoop = true;
				try {
					Thread.sleep(10);
				} catch (InterruptedException e) {
					e.printStackTrace();
				} finally {
				}
			}
		}

	}
}
