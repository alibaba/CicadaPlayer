package com.cicada.player.utils;

import android.graphics.SurfaceTexture;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;
import java.util.concurrent.CountDownLatch;

@NativeUsed
public class DecoderSurfaceTexture implements SurfaceTexture.OnFrameAvailableListener {

    private static final int CREATE_SURFACE_MSG = 12345;

    private int             mTextureId = 0;
    private long            mDecoderHandler = 0;
    private SurfaceTexture  mSurfaceTexture = null;
    private Surface         mSurface = null;
    private HandlerThread   mHandleThread = new HandlerThread("DecoderSurfaceTexture");
    private Handler         mHandler = null;
    private CountDownLatch  mCountDown = new CountDownLatch(1);

    public DecoderSurfaceTexture() {
        mHandleThread.start();
    }
    @NativeUsed
    public Surface createSurface(int id, long handler) {
        if (id <= 0) {
            return null;
        }
        mTextureId = id;
        mDecoderHandler = handler;

        try {
            mHandler = new Handler(mHandleThread.getLooper()) {
                @Override
                public void handleMessage(Message msg) {
                    // TODO Auto-generated method stub
                    if (msg.what == CREATE_SURFACE_MSG) {
                        mSurfaceTexture = new SurfaceTexture(mTextureId);
                        mSurfaceTexture.setOnFrameAvailableListener((DecoderSurfaceTexture)msg.obj);
                        mSurface = new Surface(mSurfaceTexture);
                        mCountDown.countDown();
                    } else {
                        super.handleMessage(msg);
                    }
                }
            };
        } catch (Exception e) {
            e.printStackTrace();
        }


        Message msg = new Message();
        msg.what = CREATE_SURFACE_MSG;
        msg.obj = this;
        mHandler.sendMessage(msg);

        try {
            mCountDown.await();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return mSurface;
    }
    @NativeUsed
    public void getTransformMatrix(float[] matrix) {
        if (mSurfaceTexture == null) {
            return;
        }
        mSurfaceTexture.getTransformMatrix(matrix);
    }
    @NativeUsed
    public void updateTexImage() {
        mSurfaceTexture.updateTexImage();
    }

    @NativeUsed
    public void dispose() {
        mSurface.release();
        mHandleThread.quit();
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        onFrameAvailable(mDecoderHandler);
    }

    private native void onFrameAvailable(long hander);
}
