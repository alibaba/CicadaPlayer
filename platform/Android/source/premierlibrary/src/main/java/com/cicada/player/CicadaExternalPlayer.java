package com.cicada.player;

import android.content.Context;
import android.view.Surface;

import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.Options;
import com.cicada.player.nativeclass.TrackInfo;

import java.util.ArrayList;
import java.util.List;

public abstract class CicadaExternalPlayer {


    public enum StreamType {
        ST_TYPE_UNKNOWN(-1),
        ST_TYPE_VIDEO(0),
        ST_TYPE_AUDIO(1),
        ST_TYPE_SUB(2),

        ;

        private int mValue;

        StreamType(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }
    }

    public enum DecoderType {
        DT_HARDWARE,
        DT_SOFTWARE,
        ;

        public int getValue() {
            return ordinal();
        }
    }

    public enum PropertyKey {
        PROPERTY_KEY_RESPONSE_INFO,
        PROPERTY_KEY_CONNECT_INFO,
        PROPERTY_KEY_OPEN_TIME_STR,
        PROPERTY_KEY_PROBE_STR,
        PROPERTY_KEY_VIDEO_BUFFER_LEN,
        PROPERTY_KEY_DELAY_INFO,
        PROPERTY_KEY_REMAIN_LIVE_SEG,
        PROPERTY_KEY_NETWORK_IS_CONNECTED,
    }

    public enum PlayerStatus {
        PLAYER_IDLE(0),
        PLAYER_INITIALZED(1),
        PLAYER_PREPARINIT(2),
        PLAYER_PREPARING(3),
        PLAYER_PREPARED(4),
        PLAYER_PLAYING(5),
        PLAYER_PAUSED(6),
        PLAYER_STOPPED(7),
        PLAYER_COMPLETION(8),
        PLAYER_ERROR(99),

        ;

        private int mValue = 0;

        PlayerStatus(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }
    }

    public interface OnPreparedListener {
        void onPrepared();
    }

    public interface OnLoopingStartListener {
        void onLoopingStart();
    }

    public interface OnCompletionListener {
        void onCompletion();
    }

    public interface OnFirstFrameRenderListener {
        void onFirstFrameRender();
    }

    public interface OnLoadStatusListener {
        void onLoadingStart();

        void onLoadingProgress(int progress);

        void onLoadingEnd();
    }

    public interface OnAutoPlayStartListener {
        void onAutoPlayStart();
    }

    public interface OnSeekStatusListener {
        void onSeekStart(boolean seekInCache);

        void onSeekEnd(boolean seekInCache);
    }

    public interface OnPositionUpdateListener {
        void onPositionUpdate(long position);
    }

    public interface OnBufferPositionUpdateListener {
        void onBufferPositionUpdate(long position);
    }

    public interface OnVideoSizeChangedListener {
        void onVideoSizeChanged(int width, int height);
    }

    public interface OnStatusChangedListener {
        void onStatusChanged(int from, int to);
    }

    public interface OnVideoRenderedListener {
        void onVideoRendered(long timeMs, long pts);
    }

    public interface OnErrorListener {
        void onError(int code, String msg);
    }

    public interface OnEventListener {
        void onEvent(int code, String msg);
    }

    public interface OnStreamInfoGetListener {
        void OnStreamInfoGet(MediaInfo info);
    }

    public interface OnStreamSwitchSucListener {
        void onStreamSwitchSuc(StreamType type, TrackInfo trackInfo);
    }

    public interface OnCaptureScreenListener {
        void onCaptureScreen(int width, int height, byte[] data);
    }

    public interface OnSubtitleListener {
        void onSubtitleExtAdded(int trackIndex, String url);

        void onSubtitleShow(int trackIndex, long id, String data);

        void onSubtitleHide(int trackIndex, long id);
    }

    public interface OnDRMCallback {
        byte[] onRequestProvision(String provisionUrl , byte[] data);

        byte[] onRequestKey(String licenseUrl , byte[] data);
    }

    private static List<CicadaExternalPlayer> externalPlayerList = new ArrayList<>();

    public static void registerExternalPlayer(CicadaExternalPlayer player) {
        if (player != null) {
            externalPlayerList.add(player);
        }
    }

    public static CicadaExternalPlayer isSupportExternal(Options options) {
        for (CicadaExternalPlayer player : externalPlayerList) {
            boolean support = player.isSupport(options);
            if (support) {
                return player;
            }
        }
        return null;
    }

    public abstract boolean isSupport(Options options);

    public abstract CicadaExternalPlayer create(Context context , Options options);

    public abstract void setDataSource(String url);

    public abstract void setSurface(Surface surface);

    public abstract void prepare();

    public abstract void start();

    public abstract void pause();

    public abstract void stop();

    public abstract void release();

    public abstract StreamType switchStream(int index);

    public abstract void seekTo(long seekPos, boolean accurate);

    public abstract PlayerStatus getPlayerStatus();

    public abstract long getDuration();

    public abstract long getPlayingPosition();

    public abstract long getBufferPosition();

    public abstract void mute(boolean mute);

    public abstract boolean isMute();

    public abstract void setVolume(float volume);

    public abstract float getVideoRenderFps();

    public abstract void enterBackGround(boolean back);

    public abstract void setScaleMode(CicadaPlayer.ScaleMode scaleMode);

    public abstract CicadaPlayer.ScaleMode getScaleMode();

    public abstract void setRotateMode(CicadaPlayer.RotateMode rotateMode);

    public abstract CicadaPlayer.RotateMode getRotateMode();

    public abstract void setMirrorMode(CicadaPlayer.MirrorMode mirrorMode);

    public abstract CicadaPlayer.MirrorMode getMirrorMode();

    public abstract void setVideoBackgroundColor(long color);

    public abstract int getCurrentStreamIndex(StreamType streamType);

    public abstract TrackInfo getCurrentStreamInfo(StreamType streamType);

    public abstract long getMasterClockPts();

    public abstract void setTimeout(int timeOut);

    public abstract void setDropBufferThreshold(int dropValue);

    public abstract void setDecoderType(DecoderType type);

    public abstract DecoderType getDecoderType();

    public abstract float getVolume();

    public abstract void setRefer(String refer);

    public abstract void setUserAgent(String userAgent);

    public abstract void setLooping(boolean bCirclePlay);

    public abstract boolean isLooping();

    public abstract void captureScreen();

    public abstract int getVideoWidth();

    public abstract int getVideoHeight();

    public abstract int getVideoRotation();

    public abstract String getPropertyString(PropertyKey key);

    public abstract long getPropertyInt(PropertyKey key);

    public abstract long getPropertyLong(PropertyKey key);

    public abstract float getVideoDecodeFps();

    public abstract int setOption(String key, String value);

    public abstract String getOption(String key);

    public abstract void setSpeed(float speed);

    public abstract float getSpeed();

    public abstract void addCustomHttpHeader(String httpHeader);

    public abstract void removeAllCustomHttpHeader();

    public abstract void addExtSubtitle(String uri);

    public abstract int selectExtSubtitle(int index, boolean bSelect);

    public abstract void reLoad();

    public abstract void setAutoPlay(boolean bAutoPlay);

    public abstract boolean isAutoPlay();

    public abstract int invokeComponent(String content);

    public abstract void setOnPreparedListener(OnPreparedListener onPreparedListener);

    public abstract void setOnLoopingStartListener(OnLoopingStartListener onLoopingStartListener);

    public abstract void setOnCompletionListener(OnCompletionListener onCompletionListener);

    public abstract void setOnFirstFrameRenderListener(OnFirstFrameRenderListener onFirstFrameRenderListener);

    public abstract void setOnLoadStatusListener(OnLoadStatusListener onLoadStatusListener);

    public abstract void setOnAutoPlayStartListener(OnAutoPlayStartListener onAutoPlayStartListener);

    public abstract void setOnSeekStatusListener(OnSeekStatusListener onSeekStatusListener);

    public abstract void setOnPositionUpdateListener(OnPositionUpdateListener onPositionUpdateListener);

    public abstract void setOnBufferPositionUpdateListener(OnBufferPositionUpdateListener onBufferPositionUpdateListener);

    public abstract void setOnVideoSizeChangedListener(OnVideoSizeChangedListener onVideoSizeChangedListener);

    public abstract void setOnStatusChangedListener(OnStatusChangedListener onStatusChangedListener);

    public abstract void setOnVideoRenderedListener(OnVideoRenderedListener onVideoRenderedListener);

    public abstract void setOnErrorListener(OnErrorListener onErrorListener);

    public abstract void setOnEventListener(OnEventListener onEventListener);

    public abstract void setOnStreamInfoGetListener(OnStreamInfoGetListener onStreamInfoGetListener);

    public abstract void setOnStreamSwitchSucListener(OnStreamSwitchSucListener onStreamSwitchSucListener);

    public abstract void setOnCaptureScreenListener(OnCaptureScreenListener onCaptureScreenListener);

    public abstract void setOnSubtitleListener(OnSubtitleListener onSubtitleListener);

    public abstract void setOnDrmCallback(OnDRMCallback onDRMCallback);
}
