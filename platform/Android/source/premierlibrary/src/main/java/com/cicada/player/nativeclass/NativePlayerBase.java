package com.cicada.player.nativeclass;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.net.ConnectivityManager;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Surface;

import com.cicada.player.CicadaPlayer;
import com.cicada.player.bean.ErrorCode;
import com.cicada.player.bean.ErrorInfo;
import com.cicada.player.bean.InfoBean;
import com.cicada.player.bean.InfoCode;

import java.io.File;
import java.lang.ref.WeakReference;
import java.nio.Buffer;
import java.nio.ByteBuffer;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class NativePlayerBase {

    private static final String TAG = "NativePlayerBase";
    private static String libPath = null;
    private MainHandler mCurrentThreadHandler;
    private boolean mEnableLog = false;

    void log(String tag, String msg) {
        if (mEnableLog) {
            Log.i(tag, msg);
        }
    }

    void loge(String tag, String msg) {
        if (mEnableLog) {
            Log.e(tag, msg);
        }
    }

    static {
        System.loadLibrary("alivcffmpeg");
        System.loadLibrary("CicadaPlayer");
    }


    private static class MainHandler extends Handler {
        private WeakReference<NativePlayerBase> playerWeakReference;

        public MainHandler(NativePlayerBase saasPlayer, Looper mainLooper) {
            super(mainLooper);
            playerWeakReference = new WeakReference<NativePlayerBase>(saasPlayer);
        }

        @Override
        public void handleMessage(Message msg) {

            NativePlayerBase player = playerWeakReference.get();
            if (player != null) {
                player.handleMessage(msg);
            }

            super.handleMessage(msg);

        }
    }

    private static final int UPDATE_CURRENT_POSITION = 0;

    private void handleMessage(Message msg) {
        if (msg.what == UPDATE_CURRENT_POSITION) {
            if (mOnInfoListener != null) {
                InfoBean infoBean = new InfoBean();
                infoBean.setCode(InfoCode.CurrentPosition);
                infoBean.setExtraValue(msg.arg1);
                mOnInfoListener.onInfo(infoBean);
            }
        }
    }


    private Context mContext;
    private long mNativeContext;

    protected long getNativeContext() {
        return mNativeContext;
    }

    protected void setNativeContext(long l) {
        log(TAG, "setNativeContext " + l);
        mNativeContext = l;
    }

    public NativePlayerBase(Context context) {

        mContext = context;

        if (null == libPath) {
            libPath = getUserNativeLibPath(context);
            nSetLibPath(libPath);
        }

        //保证回调的线程在创建的线程中
        log(TAG, "Looper.myLooper() == Looper.getMainLooper() ? = " + (Looper.myLooper() == Looper.getMainLooper()));

        if (Looper.myLooper() != Looper.getMainLooper()) {
            Looper.prepare();
        }
        //TODO Later : 线程的情况下回调的问题。
        mCurrentThreadHandler = new MainHandler(this, Looper.getMainLooper());

        construct(context);
    }

    private static String getUserNativeLibPath(Context context) {

        String path = context.getPackageName();
        String userPath = "/data/data/" + path + "/lib/";
        try {
            PackageInfo p = context.getPackageManager().getPackageInfo(path, 0);
            userPath = p.applicationInfo.dataDir + "/lib/";
        } catch (PackageManager.NameNotFoundException e) {
        }
        File libFile = new File(userPath);
        if (!libFile.exists() || null == libFile.listFiles()) {
            try {
                PackageInfo p = context.getPackageManager().getPackageInfo(path, 0);
                userPath = p.applicationInfo.nativeLibraryDir + "/";
            } catch (PackageManager.NameNotFoundException e) {
            }
        }

        return userPath;
    }

    private void construct(Context context) {
        nConstruct();
        if (context != null) {
            ConnectivityManager connectivityManager = (ConnectivityManager) context.getApplicationContext().getSystemService(Context.CONNECTIVITY_SERVICE);
            nSetConnectivityManager(connectivityManager);
        }
    }

    public void setSurface(Surface surface) {
        log(TAG, "setSurface surface  =  " + surface);
        nSetSurface(surface);
    }

    public void setDataSource(String url) {
        log(TAG, "setDataSource url  =  " + url);
        nSetDataSource(url);
    }

    public void prepare() {
        log(TAG, "prepare ");
        nPrepare();
    }

    public void start() {
        log(TAG, "start ");
        nStart();
    }

    public void pause() {
        log(TAG, "pause ");
        nPause();
    }

    public void stop() {
        log(TAG, "stop ");
        nStop();
    }

    public void release() {
        log(TAG, "release ");
        nRelease();
        mContext = null;
    }

    public void seekTo(long position) {
        log(TAG, "seekTo   =  " + position + " ms ");
        mCurrentThreadHandler.removeMessages(UPDATE_CURRENT_POSITION);
        nSeekTo(position, 0x10);//not Accurate
    }

    public void seekTo(long position, int mode) {
        log(TAG, "seekTo   =  " + position + " ms ");
        mCurrentThreadHandler.removeMessages(UPDATE_CURRENT_POSITION);
        nSeekTo(position, mode);
    }


    public void setMaxAccurateSeekDelta(int delta) {
        nSetMaxAccurateSeekDelta(delta);
    }

    public long getDuration() {
        long duration = nGetDuration();

        log(TAG, "getDuration =  " + duration);
        return duration;
    }

    public float getVolume() {
        float volume = nGetVolume();

        log(TAG, "getVolume =  " + volume);
        return volume;
    }

    public void setVolume(float volume) {

        log(TAG, "setVolume =  " + volume);
        nSetVolume(volume);
    }

    public void setSpeed(float speed) {
        nSetSpeed(speed);
    }

    public float getSpeed() {
        return nGetSpeed();
    }

    public void selectTrack(int trackIndex) {
        log(TAG, "selectTrack trackIndex  =  " + trackIndex);
        nSelectTrack(trackIndex);
    }

    public TrackInfo getCurrentTrackInfo(int type) {
        log(TAG, "getCurrentTrackInfo type  =  " + type);
        return (TrackInfo) nGetCurrentStreamInfo(type);
    }

    public void setLoop(boolean looping) {
        log(TAG, "setLoop = " + looping);
        nSetLoop(looping);
    }

    public boolean isLoop() {
        boolean isLoop = nIsLoop();
        log(TAG, "isLoop = " + isLoop);
        return isLoop;
    }


    public void setMute(boolean mute) {
        log(TAG, "setMute = " + mute);
        nSetMute(mute);
    }

    public boolean isMuted() {
        boolean isMute = nIsMuted();
        log(TAG, "isMuted = " + isMute);
        return isMute;
    }

    public int getVideoWidth() {
        int videoWidth = nGetVideoWidth();
        log(TAG, "getVideoWidth = " + videoWidth);
        return videoWidth;
    }

    public int getVideoHeight() {
        int videoHeight = nGetVideoHeight();
        log(TAG, "getVideoHeight = " + videoHeight);
        return videoHeight;
    }

    public float getVideoRotation() {
        int videoRotation = nGetVideoRotation();
        log(TAG, "getVideoRotation = " + videoRotation);
        return videoRotation;
    }

    public long getCurrentPosition() {
        long currentPosition = nGetCurrentPosition();
        log(TAG, "getCurrentPosition = " + currentPosition);
        return currentPosition;
    }

    public long getBufferedPosition() {
        long bufferedPosition = nGetBufferedPosition();
        log(TAG, "getBufferedPosition = " + bufferedPosition);
        return bufferedPosition;
    }

    public void enableHardwareDecoder(boolean enable) {
        log(TAG, "enableHardwareDecoder = " + enable);
        nEnableHardwareDecoder(enable);
    }

    public void setConfig(PlayerConfig config) {
        log(TAG, "setConfig = " + config);
        nSetConfig(config);
    }

    public PlayerConfig getConfig() {
        Object config = nGetConfig();
        log(TAG, "getConfig = " + config);
        if (config != null) {
            return (PlayerConfig) config;
        } else {
            return null;
        }
    }

    public void reload() {
        log(TAG, "Reload");
        nReload();
    }

    public void setScaleMode(CicadaPlayer.ScaleMode scaleMode) {
        log(TAG, "setScaleMode = " + scaleMode);
        nSetScaleMode(scaleMode.ordinal());
    }


    public void setCacheConfig(CacheConfig cacheConfig) {
        log(TAG, "setCacheConfig = " + cacheConfig.mEnable);
        nSetCacheConfig(cacheConfig);
    }


    public CicadaPlayer.ScaleMode getScaleMode() {
        int scaleMode = nGetScaleMode();
        if (scaleMode == CicadaPlayer.ScaleMode.SCALE_TO_FILL.getValue()) {
            return CicadaPlayer.ScaleMode.SCALE_TO_FILL;
        } else if (scaleMode == CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT.getValue()) {
            return CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
        } else if (scaleMode == CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL.getValue()) {
            return CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL;
        } else {
            return CicadaPlayer.ScaleMode.SCALE_TO_FILL;
        }
    }


    public void setRotateMode(CicadaPlayer.RotateMode rotateMode) {
        int rotateValue = rotateMode.getValue();
        log(TAG, "setRotateMode = " + rotateValue);
        nSetRotateMode(rotateValue);
    }

    public CicadaPlayer.RotateMode getRotateMode() {
        int rotate = nGetRotateMode();
        if (rotate == CicadaPlayer.RotateMode.ROTATE_0.getValue()) {
            return CicadaPlayer.RotateMode.ROTATE_0;
        } else if (rotate == CicadaPlayer.RotateMode.ROTATE_90.getValue()) {
            return CicadaPlayer.RotateMode.ROTATE_90;
        } else if (rotate == CicadaPlayer.RotateMode.ROTATE_180.getValue()) {
            return CicadaPlayer.RotateMode.ROTATE_180;
        } else if (rotate == CicadaPlayer.RotateMode.ROTATE_270.getValue()) {
            return CicadaPlayer.RotateMode.ROTATE_270;
        } else {
            return CicadaPlayer.RotateMode.ROTATE_0;
        }
    }

    public void setMirrorMode(CicadaPlayer.MirrorMode mirrorMode) {
        int value = mirrorMode.getValue();
        nSetMirrorMode(value);
    }

    public CicadaPlayer.MirrorMode getMirrorMode() {
        int value = nGetMirrorMode();
        if (value == CicadaPlayer.MirrorMode.MIRROR_MODE_NONE.getValue()) {
            return CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
        } else if (value == CicadaPlayer.MirrorMode.MIRROR_MODE_HORIZONTAL.getValue()) {
            return CicadaPlayer.MirrorMode.MIRROR_MODE_HORIZONTAL;
        } else if (value == CicadaPlayer.MirrorMode.MIRROR_MODE_VERTICAL.getValue()) {
            return CicadaPlayer.MirrorMode.MIRROR_MODE_VERTICAL;
        } else {
            return CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
        }
    }

    public void setTraceId(String traceId) {
        log(TAG, "setTraceId = " + traceId);
        nSetTraceID(traceId);
    }

    public void setOption(String key, String value) {
        log(TAG, "setOption = " + key + ": " + value);
        nSetOption(key, value);
    }

    public Object getOption(CicadaPlayer.Option key) {
        String optionValue = nGetOption(key.getValue());
        if (optionValue == null) {
            return null;
        }
        if (key == CicadaPlayer.Option.RenderFPS) {
            try {
                return Float.valueOf(optionValue);
            } catch (Exception e) {
                return Float.valueOf("0");
            }

        }
        return optionValue;
    }
    public void setAutoPlay(boolean autoPlay) {
        log(TAG, "setAutoPlay = " + autoPlay);
        nSetAutoPlay(autoPlay);
    }

    public boolean isAutoPlay() {
        boolean isAutoPlay = nIsAutoPlay();
        log(TAG, "isAutoPlay = " + isAutoPlay);
        return isAutoPlay;
    }


    public void snapShot() {
        log(TAG, "snapShot");
        nSnapShot();
    }

    public void addExtSubtitle(String url){
        log(TAG, "addExtSubtitle = " + url);
        nAddExtSubtitle(url);
    }

    public void selectExtSubtitle(int index, boolean select){
        log(TAG, "selectExtSubtitle  index = " + index + " , select = " + select);
        nSelectExtSubtitle(index,select);
    }

    public synchronized void setDefaultBandWidth(int bandWidth) {
        nSetDefaultBandWidth(bandWidth);
    }

    public static String getSdkVersion() {
        return nGetSdkVersion();
    }

    public static void setBlackType(int type) {
        nSetBlackType(type);
    }

    ////===============-------------------==================------------------////

    protected native void nConstruct();

    protected native void nSetConnectivityManager(Object connectManager);

    protected native void nEnableHardwareDecoder(boolean enable);

    protected native void nSetSurface(Surface surface);

    protected native void nSetDataSource(String url);

    protected native void nAddExtSubtitle(String url);

    protected native void nSelectExtSubtitle(int index, boolean select);

    protected native void nSelectTrack(int index);

    protected native void nPrepare();

    protected native void nStart();

    protected native void nPause();

    protected native void nSetVolume(float volume);

    protected native float nGetVolume();

    protected native void nSeekTo(long position, int mode);

    protected native void nSetMaxAccurateSeekDelta(int delta);

    protected native void nStop();

    protected native void nRelease();

    protected native long nGetDuration();

    protected native Object nGetCurrentStreamInfo(int type);

    protected native long nGetCurrentPosition();

    protected native long nGetBufferedPosition();

    protected native void nSetMute(boolean mute);

    protected native boolean nIsMuted();

    protected native void nSetConfig(Object config);

    protected native Object nGetConfig();

    protected native void nSetCacheConfig(Object cacheConfig);

    protected native void nReload();

    protected native void nSetScaleMode(int mode);

    protected native int nGetScaleMode();

    protected native void nSetLoop(boolean loop);

    protected native boolean nIsLoop();

    protected native int nGetVideoWidth();

    protected native int nGetVideoHeight();

    protected native int nGetVideoRotation();

    protected native void nSetRotateMode(int rotateMode);

    protected native int nGetRotateMode();

    protected native void nSetMirrorMode(int mode);

    protected native int nGetMirrorMode();

    protected native void nSetSpeed(float speed);

    protected native float nGetSpeed();

    protected native void nSetTraceID(String traceId);

    protected native void nSetLibPath(String path);

    protected native void nSetOption(String key, String value);

    protected native String nGetOption(String key);

    protected native void nSetAutoPlay(boolean autoPlay);

    protected native void nEnableVideoRenderedCallback(boolean enable);

    protected native boolean nIsAutoPlay();

    protected native void nSnapShot();

    protected native String nGetCacheFilePath(String URL);

    protected native void nSetDefaultBandWidth(int bandWidth);

    protected static native String nGetSdkVersion();

    protected static native void nSetBlackType(int type);


    //////==========--------------==================------------------================//

    private CicadaPlayer.OnVideoSizeChangedListener mOnVideoSizeChangedListener = null;
    private CicadaPlayer.OnVideoRenderedListener mOnVideoRenderedListener = null;
    private CicadaPlayer.OnInfoListener mOnInfoListener = null;
    private CicadaPlayer.OnTrackReadyListener mOnTrackReadyListener = null;
    private CicadaPlayer.OnPreparedListener mOnPreparedListener = null;
    private CicadaPlayer.OnCompletionListener mOnCompletionListener = null;
    private CicadaPlayer.OnErrorListener mOnErrorListener = null;
    private CicadaPlayer.OnRenderingStartListener mOnRenderingStartListener = null;
    private CicadaPlayer.OnTrackChangedListener mOnTrackChangedListener = null;
    private CicadaPlayer.OnLoadingStatusListener mOnLoadingStatusListener = null;
    private CicadaPlayer.OnSeekCompleteListener mOnSeekCompleteListener = null;
    private CicadaPlayer.OnSubtitleDisplayListener mOnSubtitleDisplayListener = null;
    private CicadaPlayer.OnStateChangedListener mOnStateChangedListener = null;
    private CicadaPlayer.OnSnapShotListener mOnSnapShotListener = null;

    public void setOnSubtitleDisplayListener(CicadaPlayer.OnSubtitleDisplayListener l) {
        log(TAG, "setOnSubtitleDisplayListener = " + l);
        mOnSubtitleDisplayListener = l;
    }

    public void setOnStateChangedListener(CicadaPlayer.OnStateChangedListener l) {

        log(TAG, "setOnStateChangedListener = " + l);
        mOnStateChangedListener = l;
    }

    public void setOnPreparedListener(CicadaPlayer.OnPreparedListener l) {

        log(TAG, "setOnPrepdareListener = " + l);
        mOnPreparedListener = l;
    }

    public void setOnCompletionListener(CicadaPlayer.OnCompletionListener l) {
        log(TAG, "setOnCompletionListener = " + l);
        mOnCompletionListener = l;
    }

    public void setOnInfoListener(CicadaPlayer.OnInfoListener l) {
        log(TAG, "setOnInfoListener = " + l);
        mOnInfoListener = l;
    }

    public void setOnErrorListener(CicadaPlayer.OnErrorListener l) {
        log(TAG, "setOnErrorListener = " + l);
        mOnErrorListener = l;
    }

    public void setOnRenderingStartListener(CicadaPlayer.OnRenderingStartListener l) {
        log(TAG, "setOnRenderingStartListener = " + l);
        mOnRenderingStartListener = l;
    }

    public void setOnVideoSizeChangedListener(CicadaPlayer.OnVideoSizeChangedListener l) {
        log(TAG, "setOnVideoSizeChangedListener = " + l);
        mOnVideoSizeChangedListener = l;
    }

    public void setOnVideoRenderedListener(CicadaPlayer.OnVideoRenderedListener l){
        log(TAG, "setOnVideoRenderedListener = " + l);
        mOnVideoRenderedListener = l;
        nEnableVideoRenderedCallback(l != null);
    }

    public void setOnTrackSelectRetListener(CicadaPlayer.OnTrackChangedListener l) {
        log(TAG, "setOnSwitchStreamResultListener = " + l);
        mOnTrackChangedListener = l;
    }


    public void setOnLoadingStatusListener(CicadaPlayer.OnLoadingStatusListener l) {
        log(TAG, "setOnLoadingStatusListener = " + l);
        mOnLoadingStatusListener = l;
    }

    public void setOnSeekCompleteListener(CicadaPlayer.OnSeekCompleteListener l) {
        log(TAG, "setOnSeekCompleteListener = " + l);
        mOnSeekCompleteListener = l;
    }

    public void setOnTrackInfoGetListener(CicadaPlayer.OnTrackReadyListener l) {
        log(TAG, "setOnStreamInfoGetListener = " + l);
        mOnTrackReadyListener = l;
    }

    public void setOnSnapShotListener(CicadaPlayer.OnSnapShotListener l) {
        log(TAG, "setOnSnapShotListener = " + l);
        mOnSnapShotListener = l;
    }

    /////////////////=======================///////////////

    //底层回调
    protected void onPrepared() {

        log(TAG, "onPrepared  ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnPreparedListener != null) {
                    mOnPreparedListener.onPrepared();
                }
            }
        });

    }

    protected void onCompletion() {

        log(TAG, "onCompletion  ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnCompletionListener != null) {
                    mOnCompletionListener.onCompletion();
                }
            }
        });
    }

    protected void onCircleStart() {

        log(TAG, "onCircleStart  ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnInfoListener != null) {
                    InfoBean infoBean = new InfoBean();
                    infoBean.setCode(InfoCode.LoopingStart);
                    mOnInfoListener.onInfo(infoBean);
                }
            }
        });
    }


    protected void onAutoPlayStart() {
        log(TAG, "onAutoPlayStart  ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnInfoListener != null) {

                    InfoBean infoBean = new InfoBean();
                    infoBean.setCode(InfoCode.AutoPlayStart);
                    mOnInfoListener.onInfo(infoBean);
                }
            }
        });
    }


    protected void onError(final int code, final String msg, Object extra) {

        log(TAG, "onError , " + code + " , " + msg);

        ErrorCode errorCode = ErrorCode.ERROR_UNKNOWN;
        for (ErrorCode item : ErrorCode.values()) {
            if (item.getValue() == code) {
                errorCode = item;
                break;
            }
        }

        final ErrorCode finalErrorCode = errorCode;

        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnErrorListener != null) {
                    ErrorInfo errorInfo = new ErrorInfo();
                    errorInfo.setCode(finalErrorCode);
                    errorInfo.setMsg(msg);
                    mOnErrorListener.onError(errorInfo);
                }
            }
        });
    }


    protected void onEvent(final int code, final String msg, Object extra) {
        log(TAG, "onEvent , " + code + " , " + msg);

        InfoCode infoCode = InfoCode.Unknown;
        for (InfoCode item : InfoCode.values()) {
            if (item.getValue() == code) {
                infoCode = item;
                break;
            }
        }

        final InfoCode finalInfoCode = infoCode;

        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnInfoListener != null) {
                    InfoBean infoBean = new InfoBean();
                    infoBean.setCode(finalInfoCode);
                    infoBean.setExtraMsg(msg);
                    mOnInfoListener.onInfo(infoBean);
                }
            }
        });
    }


    protected void onFirstFrameShow() {
        log(TAG, "onFirstFrameShow  ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnRenderingStartListener != null) {
                    mOnRenderingStartListener.onRenderingStart();
                }
            }
        });
    }


    protected void onVideoSizeChanged(final int width, final int height) {

        log(TAG, "onVideoSizeChanged = " + width + " , " + height);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnVideoSizeChangedListener != null) {
                    mOnVideoSizeChangedListener.onVideoSizeChanged(width, height);
                }
            }
        });
    }

    protected void onVideoRendered(final long timeMs, final long pts) {
        log(TAG, "onVideoRendered = " + timeMs + " , pts = " + pts);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnVideoRenderedListener != null) {
                    mOnVideoRenderedListener.onVideoRendered(timeMs, pts);
                }
            }
        });
    }

    protected void onStreamInfoGet(final MediaInfo mediaInfo) {
        log(TAG, "onStreamInfoGet = " + mediaInfo.getTrackInfos().size());

        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnTrackReadyListener != null) {
                    mOnTrackReadyListener.onTrackReady(mediaInfo);
                }
            }
        });
    }

    protected void onSwitchStreamSuccess(final TrackInfo newInfo) {

        log(TAG, "onSwitchStreamSuccess = " + newInfo.getType() + " , " + newInfo.getIndex());
//        mCurrentStreamInfo = newInfo;
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnTrackChangedListener != null) {
                    mOnTrackChangedListener.onChangedSuccess(newInfo);
                }
            }
        });
    }


    protected void onSwitchStreamFail(final TrackInfo targetInfo, final int code, final String msg) {
        log(TAG, "onSwitchStreamFail = " + targetInfo.getType() + " , " + targetInfo.getIndex() + " , code = " + code + " , " + msg);


        ErrorCode errorCode = ErrorCode.ERROR_UNKNOWN;
        for (ErrorCode item : ErrorCode.values()) {
            if (item.getValue() == code) {
                errorCode = item;
                break;
            }
        }

        final ErrorCode finalErrorCode = errorCode;
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {

                if (mOnTrackChangedListener != null) {
                    ErrorInfo errorInfo = new ErrorInfo();
                    errorInfo.setCode(finalErrorCode);
                    errorInfo.setMsg(code + ":" + msg);
                    mOnTrackChangedListener.onChangedFail(targetInfo, errorInfo);
                }
            }
        });
    }

    protected void onCurrentPositionUpdate(final long position) {
        log(TAG, "onCurrentPositionUpdate = " + position);

        Message msg = mCurrentThreadHandler.obtainMessage(UPDATE_CURRENT_POSITION, (int) position, 0);
        mCurrentThreadHandler.sendMessage(msg);
    }

    protected void onStatusChanged(final int newStatus, final int oldStatus) {

        log(TAG, "onStatusChanged = " + newStatus);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnStateChangedListener != null) {
                    mOnStateChangedListener.onStateChanged(newStatus);
                }
            }
        });
    }

    protected void onBufferedPositionUpdate(final long position) {

        log(TAG, "onBufferedPositionUpdate = " + position);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnInfoListener != null) {
                    InfoBean infoBean = new InfoBean();
                    infoBean.setCode(InfoCode.BufferedPosition);
                    infoBean.setExtraValue(position);
                    mOnInfoListener.onInfo(infoBean);
                }
            }
        });
    }


    protected void onLoadingStart() {
        log(TAG, "onLoadingStart ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnLoadingStatusListener != null) {
                    mOnLoadingStatusListener.onLoadingBegin();
                }
            }
        });
    }

    protected void onLoadingProgress(final float percent) {
        log(TAG, "onLoadingProgress =  " + percent);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnLoadingStatusListener != null) {
                    mOnLoadingStatusListener.onLoadingProgress((int) percent, 0);
                }
            }
        });
    }

    protected void onLoadingEnd() {
        log(TAG, "onLoadingEnd ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnLoadingStatusListener != null) {
                    mOnLoadingStatusListener.onLoadingEnd();
                }
            }
        });
    }

    protected void onSeekEnd() {
        log(TAG, "onSeekEnd ");
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnSeekCompleteListener != null) {
                    mOnSeekCompleteListener.onSeekComplete();
                }
            }
        });
    }

    protected void onShowSubtitle(final int trackIndex, final long id, final String content, final Object extra) {

        log(TAG, "onShowSubtitle  = " + id + ", " + content + " , trackIndex = " + trackIndex);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnSubtitleDisplayListener != null) {
                    mOnSubtitleDisplayListener.onSubtitleShow(trackIndex, id, content);
                }
            }
        });
    }

    protected void onSubtitleExtAdded(final int id, final String content) {

        log(TAG, "onSubtitleExtAdded  = " + id + ", " + content);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnSubtitleDisplayListener != null) {
                    mOnSubtitleDisplayListener.onSubtitleExtAdded(id, content);
                }
            }
        });
    }

    protected void onHideSubtitle(final int trackIndex, final long id) {
        log(TAG, "onHideSubtitle  = " + id + " , trackIndex = " + trackIndex);
        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {
                if (mOnSubtitleDisplayListener != null) {
                    mOnSubtitleDisplayListener.onSubtitleHide(trackIndex,id);
                }
            }
        });
    }

    protected void onCaptureScreen(final int width, final int height, final byte[] buffer) {
        log(TAG, "onCaptureScreen . width = " + width + " , height = " + height);

        Bitmap bOutput = null;
        do {
            if (width <= 0 || height <= 0 || buffer == null || buffer.length == 0) {
                loge(TAG, "onCaptureScreen .ERROR !!!  width = " + width + " , height = " + height + " , buffer = " + buffer);
                break;
            }

            try {
                bOutput = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
                Buffer src = ByteBuffer.wrap(buffer);
                bOutput.copyPixelsFromBuffer(src);
            } catch (Exception e) {
                loge(TAG, "onCaptureScreen .ERROR !!!  createBitmap exception = " + e.getMessage());
            }

        } while (false);

        final Bitmap finalBOutput = bOutput;

        mCurrentThreadHandler.post(new Runnable() {
            @Override
            public void run() {

                if (mOnSnapShotListener != null) {
                    mOnSnapShotListener.onSnapShot(finalBOutput, width, height);
                }
            }
        });

    }

    public String getCacheFilePath(String URL) {
        return nGetCacheFilePath(URL);
    }

}
