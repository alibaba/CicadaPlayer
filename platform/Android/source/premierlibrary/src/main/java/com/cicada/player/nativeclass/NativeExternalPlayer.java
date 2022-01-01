package com.cicada.player.nativeclass;

import android.content.Context;
import android.view.Surface;

import com.cicada.player.CicadaExternalPlayer;
import com.cicada.player.CicadaPlayer;
import com.cicada.player.utils.NativeUsed;

@NativeUsed
public class NativeExternalPlayer {

    static {
        System.loadLibrary("alivcffmpeg");
        System.loadLibrary("CicadaPlayer");
    }

    private CicadaExternalPlayer mExternPlayer = null;

    private long mNativeInstance = 0;

    @NativeUsed
    public static boolean isSupport(Options options) {
        CicadaExternalPlayer dummyPlayer = CicadaExternalPlayer.isSupportExternal(options);
        if (dummyPlayer != null) {
            return true;
        } else {
            return false;
        }
    }

    private static Context sContext = null;

    public static void setContext(Context context) {
        if(sContext == null && context != null) {
            sContext = context.getApplicationContext();
        }
    }

    @NativeUsed
    public void create(long nativeAddr, Options options) {
        CicadaExternalPlayer dummyPlayer = CicadaExternalPlayer.isSupportExternal(options);
        if (dummyPlayer != null) {
            mExternPlayer = dummyPlayer.create(sContext, options);
        }

        if (mExternPlayer == null) {
            return;
        }

        mNativeInstance = nativeAddr;

        mExternPlayer.setOnPreparedListener(new CicadaExternalPlayer.OnPreparedListener() {
            @Override
            public void onPrepared() {
                nativeOnPrepared(mNativeInstance);
            }
        });
        mExternPlayer.setOnLoopingStartListener(new CicadaExternalPlayer.OnLoopingStartListener() {

            @Override
            public void onLoopingStart() {
                nativeOnLoopingStart(mNativeInstance);
            }
        });

        mExternPlayer.setOnCompletionListener(new CicadaExternalPlayer.OnCompletionListener() {

            @Override
            public void onCompletion() {
                nativeOnCompletion(mNativeInstance);
            }
        });
        mExternPlayer.setOnFirstFrameRenderListener(new CicadaExternalPlayer.OnFirstFrameRenderListener() {

            @Override
            public void onFirstFrameRender() {
                nativeOnFirstFrameShow(mNativeInstance);
            }
        });
        mExternPlayer.setOnLoadStatusListener(new CicadaExternalPlayer.OnLoadStatusListener() {
            @Override
            public void onLoadingStart() {
                nativeOnLoadingStart(mNativeInstance);
            }

            @Override
            public void onLoadingProgress(int progress) {
                nativeOnLoadingProgress(mNativeInstance, progress);
            }

            @Override
            public void onLoadingEnd() {
                nativeOnLoadingEnd(mNativeInstance);
            }
        });
        mExternPlayer.setOnAutoPlayStartListener(new CicadaExternalPlayer.OnAutoPlayStartListener() {

            @Override
            public void onAutoPlayStart() {
                nativeOnAutoPlayStart(mNativeInstance);
            }
        });
        mExternPlayer.setOnSeekStatusListener(new CicadaExternalPlayer.OnSeekStatusListener() {

            @Override
            public void onSeekStart(boolean seekInCache) {
                nativeOnSeeking(mNativeInstance, seekInCache);
            }

            @Override
            public void onSeekEnd(boolean seekInCache) {
                nativeOnSeekEnd(mNativeInstance, seekInCache);
            }
        });
        mExternPlayer.setOnPositionUpdateListener(new CicadaExternalPlayer.OnPositionUpdateListener() {

            @Override
            public void onPositionUpdate(long position) {
                nativeOnPositionUpdate(mNativeInstance, position);
            }
        });
        mExternPlayer.setOnBufferPositionUpdateListener(new CicadaExternalPlayer.OnBufferPositionUpdateListener() {

            @Override
            public void onBufferPositionUpdate(long position) {
                nativeOnBufferPositionUpdate(mNativeInstance, position);
            }
        });
        mExternPlayer.setOnVideoSizeChangedListener(new CicadaExternalPlayer.OnVideoSizeChangedListener() {
            @Override
            public void onVideoSizeChanged(int width, int height) {
                nativeOnVideoSizeChanged(mNativeInstance, width, height);
            }
        });
        mExternPlayer.setOnStatusChangedListener(new CicadaExternalPlayer.OnStatusChangedListener() {
            @Override
            public void onStatusChanged(int from, int to) {
                nativeOnStatusChanged(mNativeInstance, from, to);
            }
        });
        mExternPlayer.setOnVideoRenderedListener(new CicadaExternalPlayer.OnVideoRenderedListener() {

            @Override
            public void onVideoRendered(long timeMs, long pts) {
                nativeOnVideoRendered(mNativeInstance, timeMs, pts);
            }
        });
        mExternPlayer.setOnErrorListener(new CicadaExternalPlayer.OnErrorListener() {

            @Override
            public void onError(int code, String msg) {
                nativeOnErrorCallback(mNativeInstance, code, msg);
            }
        });
        mExternPlayer.setOnEventListener(new CicadaExternalPlayer.OnEventListener() {

            @Override
            public void onEvent(int code, String msg) {
                nativeOnEventCallback(mNativeInstance, code, msg);
            }
        });
        mExternPlayer.setOnStreamInfoGetListener(new CicadaExternalPlayer.OnStreamInfoGetListener() {

            @Override
            public void OnStreamInfoGet(MediaInfo info) {
                nativeOnStreamInfoGet(mNativeInstance, info);
            }
        });
        mExternPlayer.setOnStreamSwitchSucListener(new CicadaExternalPlayer.OnStreamSwitchSucListener() {

            @Override
            public void onStreamSwitchSuc(CicadaExternalPlayer.StreamType type, TrackInfo trackInfo) {
                nativeOnStreamSwitchSuc(mNativeInstance, type.ordinal(), trackInfo);
            }
        });
        mExternPlayer.setOnCaptureScreenListener(new CicadaExternalPlayer.OnCaptureScreenListener() {
            @Override
            public void onCaptureScreen(int width, int height, byte[] data) {
                nativeOnCaptureScreen(mNativeInstance, width, height, data);
            }
        });
        mExternPlayer.setOnSubtitleListener(new CicadaExternalPlayer.OnSubtitleListener() {
            @Override
            public void onSubtitleExtAdded(int trackIndex, String url) {
                nativeOnSubtitleExtAdd(mNativeInstance, trackIndex, url);
            }

            @Override
            public void onSubtitleShow(int trackIndex, long id, String data) {
                nativeOnSubtitleShow(mNativeInstance, trackIndex, data.getBytes());
            }

            @Override
            public void onSubtitleHide(int trackIndex, long id) {
                nativeOnSubtitleHide(mNativeInstance, trackIndex, null);
            }
        });
        mExternPlayer.setOnDrmCallback(new CicadaExternalPlayer.OnDRMCallback() {

            @Override
            public byte[] onRequestProvision(String provisionUrl, byte[] data) {
                return nativeOnRequestProvision(mNativeInstance ,provisionUrl , data);
            }

            @Override
            public byte[] onRequestKey(String licenseUrl, byte[] data) {
                return nativeOnRequestKey(mNativeInstance , licenseUrl , data);
            }
        });
    }


    @NativeUsed
    CicadaExternalPlayer.StreamType SwitchStream(int index) {
        if (mExternPlayer == null) {
            return CicadaExternalPlayer.StreamType.ST_TYPE_UNKNOWN;
        }
        return mExternPlayer.switchStream(index);
    }


    private void setScaleMode(int scaleMode) {
        if (mExternPlayer == null) {
            return;
        }
        CicadaPlayer.ScaleMode targetMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
        if (scaleMode == 0) {
            targetMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
        } else if (scaleMode == 1) {
            targetMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL;
        } else if (scaleMode == 2) {
            targetMode = CicadaPlayer.ScaleMode.SCALE_TO_FILL;
        }
        mExternPlayer.setScaleMode(targetMode);
    }


    private void setRotateMode(int mode) {
        if (mExternPlayer == null) {
            return;
        }
        CicadaPlayer.RotateMode rotateMode = CicadaPlayer.RotateMode.ROTATE_0;
        if (mode == 90) {
            rotateMode = CicadaPlayer.RotateMode.ROTATE_90;
        } else if (mode == 180) {
            rotateMode = CicadaPlayer.RotateMode.ROTATE_180;
        } else if (mode == 270) {
            rotateMode = CicadaPlayer.RotateMode.ROTATE_270;
        } else if (mode == 0) {
            rotateMode = CicadaPlayer.RotateMode.ROTATE_0;
        }
        mExternPlayer.setRotateMode(rotateMode);
    }


    private void setMirrorMode(int mode) {
        if (mExternPlayer == null) {
            return;
        }
        CicadaPlayer.MirrorMode mirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
        if (mode == 0) {
            mirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
        } else if (mode == 1) {
            mirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_HORIZONTAL;
        } else if (mode == 2) {
            mirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_VERTICAL;
        }
        mExternPlayer.setMirrorMode(mirrorMode);
    }

    private int getCurrentStreamIndex(int type) {
        if (mExternPlayer == null) {
            return -1;
        }
        CicadaExternalPlayer.StreamType streamType = CicadaExternalPlayer.StreamType.ST_TYPE_UNKNOWN;
        if (type == 0) {
            streamType = CicadaExternalPlayer.StreamType.ST_TYPE_VIDEO;
        } else if (type == 1) {
            streamType = CicadaExternalPlayer.StreamType.ST_TYPE_AUDIO;
        } else if (type == 2) {
            streamType = CicadaExternalPlayer.StreamType.ST_TYPE_SUB;
        }
        return mExternPlayer.getCurrentStreamIndex(streamType);
    }

    public Object getCurrentStreamInfo(int type) {
        if (mExternPlayer == null) {
            return null;
        }

        CicadaExternalPlayer.StreamType streamType = CicadaExternalPlayer.StreamType.ST_TYPE_UNKNOWN;
        if (type == 0) {
            streamType = CicadaExternalPlayer.StreamType.ST_TYPE_VIDEO;
        } else if (type == 1) {
            streamType = CicadaExternalPlayer.StreamType.ST_TYPE_AUDIO;
        } else if (type == 2) {
            streamType = CicadaExternalPlayer.StreamType.ST_TYPE_SUB;
        }

        return mExternPlayer.getCurrentStreamInfo(streamType);
    }

    private void setDecoderType(int decoderType) {
        if (mExternPlayer == null) {
            return;
        }
        CicadaExternalPlayer.DecoderType type = CicadaExternalPlayer.DecoderType.DT_SOFTWARE;
        if (decoderType == 0) {
            type = CicadaExternalPlayer.DecoderType.DT_HARDWARE;
        } else if (decoderType == 1) {
            type = CicadaExternalPlayer.DecoderType.DT_HARDWARE;
        }
        mExternPlayer.setDecoderType(type);
    }

    private String getPropertyString(CicadaExternalPlayer.PropertyKey key) {
        if (mExternPlayer == null) {
            return null;
        }
        return mExternPlayer.getPropertyString(key);
    }

    private long getPropertyInt(CicadaExternalPlayer.PropertyKey key) {
        if (mExternPlayer == null) {
            return 0;
        }
        return mExternPlayer.getPropertyInt(key);
    }

    private long getPropertyLong(CicadaExternalPlayer.PropertyKey key) {
        if (mExternPlayer == null) {
            return 0;
        }
        return mExternPlayer.getPropertyLong(key);
    }

    private int setOption(String key, String value) {
        if (mExternPlayer == null) {
            return -1;
        }
        return mExternPlayer.setOption(key, value);
    }

    private String getOption(String key) {
        if (mExternPlayer == null) {
            return null;
        }
        return mExternPlayer.getOption(key);
    }


    private int selectExtSubtitle(int index, boolean bSelect) {
        if (mExternPlayer == null) {
            return -1;
        }
        return mExternPlayer.selectExtSubtitle(index, bSelect);
    }

//        //TODO
//    public int getCurrentStreamMeta(Stream_meta *meta, StreamType type) {
////        if (mExternPlayer == null) {
////            return;
////        }
////        mExternPlayer.setTimeout(timeOut);
//    }


    private int invokeComponent(String content) {
        if (mExternPlayer == null) {
            return -1;
        }
        return mExternPlayer.invokeComponent(content);
    }

    //R: return ,P:param D:default value
    public void callRvPv(String name) {
        if (mExternPlayer == null) {
            return;
        }

        if ("Release".equals(name)) {
            mExternPlayer.release();
            mNativeInstance = 0;
            mExternPlayer = null;
        } else if ("Prepare".equals(name)) {
            mExternPlayer.prepare();
        } else if ("Start".equals(name)) {
            mExternPlayer.start();
        } else if ("Pause".equals(name)) {
            mExternPlayer.pause();
        } else if ("CaptureScreen".equals(name)) {
            mExternPlayer.captureScreen();
        } else if ("reLoad".equals(name)) {
            mExternPlayer.reLoad();
        } else if ("RemoveAllCustomHttpHeader".equals(name)) {
            mExternPlayer.removeAllCustomHttpHeader();
        }
    }

    public void callRvPf(String name, float value) {
        if (mExternPlayer == null) {
            return;
        }
        if ("SetVolume".equals(name)) {
            mExternPlayer.setVolume(value);
        }
        if ("setSpeed".equals(name)) {
            mExternPlayer.setSpeed(value);
        }
    }

    public void callRvPi(String name, int value) {
        if (mExternPlayer == null) {
            return;
        }
        if ("SetVolume".equals(name)) {
            mExternPlayer.setVolume(value);
        } else if ("SetScaleMode".equals(name)) {
            setScaleMode(value);
        } else if ("SetRotateMode".equals(name)) {
            setRotateMode(value);
        } else if ("SetMirrorMode".equals(name)) {
            setMirrorMode(value);
        } else if ("SetTimeout".equals(name)) {
            mExternPlayer.setTimeout(value);
        } else if ("SetDropBufferThreshold".equals(name)) {
            mExternPlayer.setDropBufferThreshold(value);
        } else if ("SetDecoderType".equals(name)) {
            setDecoderType(value);
        }
    }


    public void callRvPs(String name, String value) {
        if (mExternPlayer == null) {
            return;
        }
        if ("SetDataSource".equals(name)) {
            mExternPlayer.setDataSource(value);
        } else if ("addExtSubtitle".equals(name)) {
            mExternPlayer.addExtSubtitle(value);
        } else if ("AddCustomHttpHeader".equals(name)) {
            mExternPlayer.addCustomHttpHeader(value);
        } else if ("SetUserAgent".equals(name)) {
            mExternPlayer.setUserAgent(value);
        } else if ("SetRefer".equals(name)) {
            mExternPlayer.setRefer(value);
        }
    }

    public void callRvPlb(String name, long lv, boolean bv) {
        if (mExternPlayer == null) {
            return;
        }
        if ("SeekTo".equals(name)) {
            mExternPlayer.seekTo(lv, bv);
        } else if ("SetVideoBackgroundColor".equals(name)) {
            mExternPlayer.setVideoBackgroundColor(lv);
        } else if ("Mute".equals(name)) {
            mExternPlayer.mute(bv);
        } else if ("EnterBackGround".equals(name)) {
            mExternPlayer.enterBackGround(bv);
        } else if ("SetLooping".equals(name)) {
            mExternPlayer.setLooping(bv);
        } else if ("SetAutoPlay".equals(name)) {
            mExternPlayer.setAutoPlay(bv);
        } else if("selectExtSubtitle".equals(name)){
            mExternPlayer.selectExtSubtitle((int)lv, bv);
        }
    }

    public void callRvPo(String name, Object value) {
        if (mExternPlayer == null) {
            return;
        }
        if ("SetView".equals(name)) {
            mExternPlayer.setSurface((Surface) value);
        }
    }

    public Object callRoPi(String name, int value) {
        if (mExternPlayer == null) {
            return null;
        }

        if ("GetCurrentStreamInfo".equals(name)) {
            return getCurrentStreamInfo(value);
        }

        return null;
    }


    public boolean callRbPvD(String name, boolean defaultValue) {
        if (mExternPlayer == null) {
            return defaultValue;
        }

        if ("IsMute".equals(name)) {
            return mExternPlayer.isMute();
        } else if ("isLooping".equals(name)) {
            return mExternPlayer.isLooping();
        } else if ("IsAutoPlay".equals(name)) {
            return mExternPlayer.isAutoPlay();
        }


        return defaultValue;
    }

    public float callRfPvD(String name, float defaultValue) {
        if (mExternPlayer == null) {
            return defaultValue;
        }

        if ("GetVideoRenderFps".equals(name)) {
            return mExternPlayer.getVideoRenderFps();
        } else if ("GetVolume".equals(name)) {
            return mExternPlayer.getVolume();
        } else if ("getSpeed".equals(name)) {
            return mExternPlayer.getSpeed();
        } else if ("GetVideoDecodeFps".equals(name)) {
            return mExternPlayer.getVideoDecodeFps();
        }

        return defaultValue;
    }

    public int callRiPvD(String name, int defaultValue) {
        if (mExternPlayer == null) {
            return defaultValue;
        }

        if ("Stop".equals(name)) {
            mExternPlayer.stop();
            return 0;
        } else if ("GetScaleMode".equals(name)) {
            return mExternPlayer.getScaleMode().getValue();
        } else if ("GetRotateMode".equals(name)) {
            return mExternPlayer.getRotateMode().getValue();
        } else if ("GetMirrorMode".equals(name)) {
            return mExternPlayer.getMirrorMode().getValue();
        } else if ("GetDecoderType".equals(name)) {
            return mExternPlayer.getDecoderType().getValue();
        } else if ("getVideoWidth".equals(name)) {
            return mExternPlayer.getVideoWidth();
        } else if ("getVideoHeight".equals(name)) {
            return mExternPlayer.getVideoHeight();
        } else if ("GetVideoRotation".equals(name)) {
            return mExternPlayer.getVideoRotation();
        }

        return defaultValue;
    }

    public int callRiPiD(String name, int iv, int defaultValue) {
        if (mExternPlayer == null) {
            return defaultValue;
        }

        if ("GetCurrentStreamIndex".equals(name)) {
            return getCurrentStreamIndex(iv);
        } else if ("SwitchStream".equals(name)) {
            return SwitchStream(iv).getValue();
        }
        return defaultValue;
    }


    public long callRlPvD(String name, long defaultValue) {
        if (mExternPlayer == null) {
            return defaultValue;
        }

        if ("GetDuration".equals(name)) {
            return mExternPlayer.getDuration();
        } else if ("GetPlayingPosition".equals(name)) {
            return mExternPlayer.getPlayingPosition();
        } else if ("GetBufferPosition".equals(name)) {
            return mExternPlayer.getBufferPosition();
        } else if ("GetMasterClockPts".equals(name)) {
            return mExternPlayer.getMasterClockPts();
        }

        return defaultValue;
    }

    //=================================

    private native void nativeOnPrepared(long mNativeInstance);

    private native void nativeOnLoopingStart(long mNativeInstance);

    private native void nativeOnCompletion(long mNativeInstance);

    private native void nativeOnFirstFrameShow(long mNativeInstance);

    private native void nativeOnLoadingStart(long mNativeInstance);

    private native void nativeOnLoadingEnd(long mNativeInstance);

    private native void nativeOnAutoPlayStart(long mNativeInstance);

    private native void nativeOnSeeking(long mNativeInstance, boolean seekInCache);

    private native void nativeOnSeekEnd(long mNativeInstance, boolean seekInCache);

    private native void nativeOnPositionUpdate(long mNativeInstance, long position);

    private native void nativeOnBufferPositionUpdate(long mNativeInstance, long bufferPosition);

    private native void nativeOnLoadingProgress(long mNativeInstance, long progress);

    private native void nativeOnVideoSizeChanged(long mNativeInstance, int width, int height);

    private native void nativeOnStatusChanged(long mNativeInstance, int from, int to);

    private native void nativeOnVideoRendered(long mNativeInstance, long timeMs, long pts);

    private native void nativeOnErrorCallback(long mNativeInstance, long code, String msg);

    private native void nativeOnEventCallback(long mNativeInstance, long code, String msg);

    private native void nativeOnStreamInfoGet(long mNativeInstance, MediaInfo info);

    private native void nativeOnStreamSwitchSuc(long mNativeInstance, int type, TrackInfo info);

    private native void nativeOnCaptureScreen(long mNativeInstance, int width, int height, byte[] datas);

    private native void nativeOnSubtitleHide(long mNativeInstance, long index, byte[] content);

    private native void nativeOnSubtitleShow(long mNativeInstance, long index, byte[] content);

    private native void nativeOnSubtitleExtAdd(long mNativeInstance, long index, String url);

    private native byte[] nativeOnRequestProvision(long mNativeInstance , String provisionUrl , byte[] data);

    private native byte[] nativeOnRequestKey(long mNativeInstance , String licenseUrl , byte[] data);
}
