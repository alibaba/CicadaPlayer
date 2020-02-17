package com.cicada.player.utils;

import android.os.Build;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.support.annotation.RequiresApi;
import android.util.Log;
import android.view.Choreographer;

@RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
@NativeUsed
public class VsyncTimer {

    private static final String TAG = VsyncTimer.class.getSimpleName();

    private static int WHAT_INIT = 10000;
    private static int WHAT_START = 10001;
    private static int WHAT_PAUSE = 10002;
    private static int WHAT_DESTROY = 10003;

    private long mNativePtr;
    private HandlerThread mTimerThread;
    private Handler mTimerHandler;

    private final Object lockObj = new Object();

    private Choreographer.FrameCallback mFrameCallback = new Choreographer.FrameCallback() {
        @Override
        public void doFrame(long frameTimeNanos) {
            onVsync(mNativePtr, frameTimeNanos);
            Choreographer.getInstance().postFrameCallback(this);
        }
    };

    public VsyncTimer(long nativePtr) {
        mNativePtr = nativePtr;
        mTimerThread = new HandlerThread(TAG);
        mTimerThread.start();
        mTimerHandler = new Handler(mTimerThread.getLooper()) {
            @Override
            public void handleMessage(Message msg) {
                if (msg.what == WHAT_INIT) {
                    onInit(mNativePtr);
                } else if (msg.what == WHAT_START) {
                    Choreographer.getInstance().postFrameCallback(mFrameCallback);
                } else if (msg.what == WHAT_PAUSE) {
                    Choreographer.getInstance().removeFrameCallback(mFrameCallback);
                } else if (msg.what == WHAT_DESTROY) {
                    Choreographer.getInstance().removeFrameCallback(mFrameCallback);
                    onDestroy(mNativePtr);
                    mTimerThread.quit();
                    mNativePtr = 0;

                    synchronized (lockObj) {
                        lockObj.notifyAll();
                    }
                }
            }
        };


        mTimerHandler.sendEmptyMessage(WHAT_INIT);
    }

    public void start() {
        mTimerHandler.sendEmptyMessage(WHAT_START);
    }

    public void pause() {
        mTimerHandler.sendEmptyMessage(WHAT_PAUSE);
    }

    public void destroy() {
        synchronized (lockObj) {
            mTimerHandler.sendEmptyMessage(WHAT_DESTROY);
            try {
                lockObj.wait();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private native int onInit(long nativePtr);

    private native int onVsync(long nativePtr, long frameTimeNanos);

    private native void onDestroy(long nativePtr);
}
