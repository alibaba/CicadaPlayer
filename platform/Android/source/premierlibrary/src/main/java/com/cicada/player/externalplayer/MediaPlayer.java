package com.cicada.player.externalplayer;

import android.content.Context;
import android.media.PlaybackParams;
import android.media.TimedText;
import android.net.Uri;
import android.os.Build;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.view.Surface;

import com.cicada.player.CicadaExternalPlayer;
import com.cicada.player.CicadaPlayer;
import com.cicada.player.bean.ErrorCode;
import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.Options;
import com.cicada.player.nativeclass.TrackInfo;
import com.cicada.player.utils.Logger;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

import static android.media.MediaPlayer.SEEK_CLOSEST;

public class MediaPlayer extends CicadaExternalPlayer {

    private static final String PLAYER_NAME = "MediaPlayer";
    private static final String TAG = PLAYER_NAME;

    private final int TIMER_WHAT_HALF_SECOND = 1000;

    private Context mContext = null;
    private android.media.MediaPlayer mSystemMediaPlayer = null;
    private android.media.MediaPlayer.TrackInfo[] mTrackInfos = null;
    private Handler timer = null;
    private PlayerStatus mLastPlayerStatus = PlayerStatus.PLAYER_IDLE;
    private String mUrl = null;
    private long mBufferPosition = 0;
    private float lastVolume = 1.0f;
    private boolean isMute = false;
    private String mRefer = null;
    private String mUserAgent = null;
    private float mSpeed = 1.0f;
    private boolean mAutoPlay = false;
    private OnPreparedListener mOutOnPreparedListener = null;
    private OnLoopingStartListener mOutOnLoopingStartListener = null;
    private OnCompletionListener mOutOnCompletionListener = null;
    private OnFirstFrameRenderListener mOutOnFirstFrameRenderListener = null;
    private OnLoadStatusListener mOutOnLoadStatusListener = null;
    private OnAutoPlayStartListener mOutOnAutoPlayStartListener = null;
    private OnSeekStatusListener mOutOnSeekStatusListener = null;
    private OnPositionUpdateListener mOutOnPositionUpdateListener = null;
    private OnBufferPositionUpdateListener mOutOnBufferPositionUpdateListener = null;
    private OnVideoSizeChangedListener mOutOnVideoSizeChangedListener = null;
    private OnStatusChangedListener mOutOnStatusChangedListener = null;
    private OnVideoRenderedListener mOutOnVideoRenderedListener = null;
    private OnErrorListener mOutOnErrorListener = null;
    private OnEventListener mOutOnEventListener = null;
    private OnStreamInfoGetListener mOutOnStreamInfoGetListener = null;
    private OnStreamSwitchSucListener mOutOnStreamSwitchSucListener = null;
    private OnCaptureScreenListener mOutOnCaptureScreenListener = null;
    private OnSubtitleListener mOutOnSubtitleListener = null;
    private OnDRMCallback mOutOnDRMCallback = null;

    public MediaPlayer() {
    }

    private MediaPlayer(Context context, Options options) {
        mContext = context;

        Looper looper = Looper.myLooper();
        if (looper == null) {
            looper = Looper.getMainLooper();
        }

        timer = new Handler(looper) {
            @Override
            public void handleMessage(Message msg) {
                if (msg.what == TIMER_WHAT_HALF_SECOND) {

                    if (mLastPlayerStatus.getValue() >= PlayerStatus.PLAYER_PREPARED.getValue()
                            && mLastPlayerStatus.getValue() <= PlayerStatus.PLAYER_STOPPED.getValue()) {
                        if (mOutOnBufferPositionUpdateListener != null) {
                            mOutOnBufferPositionUpdateListener.onBufferPositionUpdate(getBufferPosition());
                        }
                        if (mOutOnPositionUpdateListener != null) {
                            mOutOnPositionUpdateListener.onPositionUpdate(getPlayingPosition());
                        }
                    }

                    sendHalfSecondTimer();
                }

                super.handleMessage(msg);

            }
        };


        mSystemMediaPlayer = new android.media.MediaPlayer();
        mSystemMediaPlayer.setOnBufferingUpdateListener(new android.media.MediaPlayer.OnBufferingUpdateListener() {
            @Override
            public void onBufferingUpdate(android.media.MediaPlayer mediaPlayer, int i) {
                mBufferPosition = (long) (i * getDuration() / 100.0f);
            }
        });
        mSystemMediaPlayer.setOnCompletionListener(new android.media.MediaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion(android.media.MediaPlayer mediaPlayer) {
                if (mOutOnCompletionListener != null) {
                    mOutOnCompletionListener.onCompletion();
                }
                changePlayerStatus(PlayerStatus.PLAYER_COMPLETION);
            }
        });
        mSystemMediaPlayer.setOnErrorListener(new android.media.MediaPlayer.OnErrorListener() {
            @Override
            public boolean onError(android.media.MediaPlayer mediaPlayer, int what, int extra) {
                if (mOutOnErrorListener != null) {
                    mOutOnErrorListener.onError(ErrorCode.ERROR_UNKNOWN.getValue(), "what=" + what + ", extra=" + extra);
                }
                changePlayerStatus(PlayerStatus.PLAYER_ERROR);
                return true;
            }
        });
        mSystemMediaPlayer.setOnInfoListener(new android.media.MediaPlayer.OnInfoListener() {
            @Override
            public boolean onInfo(android.media.MediaPlayer mediaPlayer, int what, int extra) {
                if (what == android.media.MediaPlayer.MEDIA_INFO_BUFFERING_END) {
                    if (mOutOnLoadStatusListener != null) {
                        mOutOnLoadStatusListener.onLoadingEnd();
                    }
                } else if (what == android.media.MediaPlayer.MEDIA_INFO_BUFFERING_START) {
                    if (mOutOnLoadStatusListener != null) {
                        mOutOnLoadStatusListener.onLoadingStart();
                    }
                } else if (what == android.media.MediaPlayer.MEDIA_INFO_VIDEO_RENDERING_START) {
                    if (mOutOnFirstFrameRenderListener != null) {
                        mOutOnFirstFrameRenderListener.onFirstFrameRender();
                    }
                } else if (what == android.media.MediaPlayer.MEDIA_INFO_UNSUPPORTED_SUBTITLE ||
                        what == android.media.MediaPlayer.MEDIA_INFO_SUBTITLE_TIMED_OUT) {
                }
                return false;
            }
        });
        mSystemMediaPlayer.setOnPreparedListener(new android.media.MediaPlayer.OnPreparedListener() {
            @Override
            public void onPrepared(android.media.MediaPlayer mediaPlayer) {
                mTrackInfos = mSystemMediaPlayer.getTrackInfo();
                notifyStreamGet();

                if (mAutoPlay) {
                    if (mOutOnAutoPlayStartListener != null) {
                        mOutOnAutoPlayStartListener.onAutoPlayStart();
                    }

                    start();
                } else {
                    changePlayerStatus(PlayerStatus.PLAYER_PREPARED);
                    if (mOutOnPreparedListener != null) {
                        mOutOnPreparedListener.onPrepared();
                    }
                }
            }
        });
        mSystemMediaPlayer.setOnSeekCompleteListener(new android.media.MediaPlayer.OnSeekCompleteListener() {
            @Override
            public void onSeekComplete(android.media.MediaPlayer mediaPlayer) {
                if (mOutOnSeekStatusListener != null) {
                    mOutOnSeekStatusListener.onSeekEnd(false);
                }
            }
        });
        mSystemMediaPlayer.setOnTimedTextListener(new android.media.MediaPlayer.OnTimedTextListener() {
            @Override
            public void onTimedText(android.media.MediaPlayer mediaPlayer, TimedText timedText) {
            }
        });
        mSystemMediaPlayer.setOnVideoSizeChangedListener(new android.media.MediaPlayer.OnVideoSizeChangedListener() {
            @Override
            public void onVideoSizeChanged(android.media.MediaPlayer mediaPlayer, int width, int height) {
                if (mOutOnVideoSizeChangedListener != null) {
                    mOutOnVideoSizeChangedListener.onVideoSizeChanged(width, height);
                }
            }
        });

    }

    private void notifyStreamGet() {
        if (mOutOnStreamInfoGetListener != null && mTrackInfos != null) {
            MediaInfo mediaInfo = new MediaInfo();
            TrackInfo[] trackInfoList = new TrackInfo[mTrackInfos.length];
            for (int index = 0; index < mTrackInfos.length; index++) {
                android.media.MediaPlayer.TrackInfo trackInfo = mTrackInfos[index];
                trackInfoList[index] = convert(trackInfo, index);
            }
            mediaInfo.setTrackInfos(trackInfoList);
            mOutOnStreamInfoGetListener.OnStreamInfoGet(mediaInfo);
        }
    }

    private void sendHalfSecondTimer() {
        timer.removeMessages(TIMER_WHAT_HALF_SECOND);
        if (mLastPlayerStatus.getValue() >= PlayerStatus.PLAYER_PREPARED.getValue()
                && mLastPlayerStatus.getValue() <= PlayerStatus.PLAYER_STOPPED.getValue()) {
            timer.sendEmptyMessageDelayed(TIMER_WHAT_HALF_SECOND, 500);
        }
    }

    private void changePlayerStatus(PlayerStatus status) {
        if (mLastPlayerStatus != status) {
            if (mOutOnStatusChangedListener != null) {
                mOutOnStatusChangedListener.onStatusChanged(mLastPlayerStatus.getValue(), status.getValue());
            }
            mLastPlayerStatus = status;
        }

        sendHalfSecondTimer();
    }

    @Override
    public boolean isSupport(Options options) {
        if (options == null) {
            return false;
        }

        String name = options.get("name");
        if (PLAYER_NAME.equals(name)) {
            return true;
        }

        return false;
    }

    @Override
    public CicadaExternalPlayer create(Context context, Options options) {
        return new MediaPlayer(context, options);
    }

    @Override
    public void setDataSource(String url) {
        mUrl = url;
        if (mSystemMediaPlayer != null) {
            changePlayerStatus(PlayerStatus.PLAYER_INITIALZED);
        }
    }

    private void updateDataSource() {
        if (mSystemMediaPlayer == null) {
            return;
        }


        Map<String, String> headers = new HashMap<>();
        if (mRefer != null) {
            headers.put("Referer", mRefer);
        }

        if (mUserAgent != null) {
            headers.put("User-Agent", mUserAgent);
        }

        headers.putAll(mCustomHeaders);

        Uri uri = Uri.parse(mUrl);

        try {
            mSystemMediaPlayer.setDataSource(mContext, uri, headers);
        } catch (IOException e) {
            e.printStackTrace();
            if (mOutOnErrorListener != null) {
                mOutOnErrorListener.onError(ErrorCode.ERROR_GENERAL_EIO.getValue(), "set dataSource error :" + e.getMessage());
            }
        }
    }

    @Override
    public void setSurface(Surface surface) {
        if (mSystemMediaPlayer != null) {
            mSystemMediaPlayer.setSurface(surface);
        }
    }

    @Override
    public void prepare() {
        if (mSystemMediaPlayer != null) {
            mSystemMediaPlayer.reset();
            updateDataSource();
            changePlayerStatus(PlayerStatus.PLAYER_PREPARING);
            mSystemMediaPlayer.prepareAsync();
        }
    }

    @Override
    public void start() {
        if (mSystemMediaPlayer != null) {
            mSystemMediaPlayer.start();
            changePlayerStatus(PlayerStatus.PLAYER_PLAYING);
        }
    }

    @Override
    public void pause() {
        if (mSystemMediaPlayer != null) {
            mSystemMediaPlayer.pause();
            changePlayerStatus(PlayerStatus.PLAYER_PAUSED);
        }
    }

    @Override
    public void stop() {
        if (mSystemMediaPlayer != null) {
            if (mLastPlayerStatus != PlayerStatus.PLAYER_STOPPED) {
                mSystemMediaPlayer.stop();
                changePlayerStatus(PlayerStatus.PLAYER_STOPPED);
            }
        }
    }

    @Override
    public void release() {
        if (mSystemMediaPlayer != null) {
            mSystemMediaPlayer.release();
        }
        mSystemMediaPlayer = null;
    }

    @Override
    public StreamType switchStream(int index) {
        if (mSystemMediaPlayer != null) {
            mSystemMediaPlayer.selectTrack(index);
            if (mTrackInfos == null) {
                return StreamType.ST_TYPE_UNKNOWN;
            }
            android.media.MediaPlayer.TrackInfo trackInfo = mTrackInfos[index];
            int trackType = trackInfo.getTrackType();
            if (trackType == android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_AUDIO) {
                return StreamType.ST_TYPE_AUDIO;
            } else if (trackType == android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_SUBTITLE) {
                return StreamType.ST_TYPE_SUB;
            } else if (trackType == android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_VIDEO) {
                return StreamType.ST_TYPE_VIDEO;
            } else if (trackType == android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_UNKNOWN) {
                return StreamType.ST_TYPE_UNKNOWN;
            } else {
                return StreamType.ST_TYPE_UNKNOWN;
            }
        } else {
            return StreamType.ST_TYPE_UNKNOWN;
        }
    }

    @Override
    public void seekTo(long seekPos, boolean accurate) {
        if (mSystemMediaPlayer != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O && accurate) {
                mSystemMediaPlayer.seekTo(seekPos, SEEK_CLOSEST);
            } else {
                mSystemMediaPlayer.seekTo((int) seekPos);
            }

            if (mOutOnSeekStatusListener != null) {
                mOutOnSeekStatusListener.onSeekStart(false);
            }
        }
    }

    @Override
    public PlayerStatus getPlayerStatus() {
        return mLastPlayerStatus;
    }

    @Override
    public long getDuration() {
        if (mSystemMediaPlayer != null) {
            int duration = 0;
            if (mLastPlayerStatus != PlayerStatus.PLAYER_ERROR) {
                duration = mSystemMediaPlayer.getDuration();
            }
            return Math.max(duration, 0);
        }
        return 0;
    }

    @Override
    public long getPlayingPosition() {
        if (mSystemMediaPlayer != null) {
            return mSystemMediaPlayer.getCurrentPosition();
        }
        return 0;
    }

    @Override
    public long getBufferPosition() {
        return mBufferPosition;
    }

    @Override
    public void mute(boolean mute) {
        if (mSystemMediaPlayer != null) {
            isMute = mute;
            if (mute) {
                mSystemMediaPlayer.setVolume(0, 0);
            } else {
                mSystemMediaPlayer.setVolume(lastVolume, lastVolume);
            }
        }
    }

    @Override
    public boolean isMute() {
        return isMute;
    }

    @Override
    public float getVideoRenderFps() {
        return 0;
    }

    @Override
    public void enterBackGround(boolean back) {

    }

    @Override
    public CicadaPlayer.ScaleMode getScaleMode() {
        return CicadaPlayer.ScaleMode.SCALE_TO_FILL;
    }

    @Override
    public void setScaleMode(CicadaPlayer.ScaleMode scaleMode) {
    }

    @Override
    public CicadaPlayer.RotateMode getRotateMode() {
        return CicadaPlayer.RotateMode.ROTATE_0;
    }

    @Override
    public void setRotateMode(CicadaPlayer.RotateMode rotateMode) {

    }

    @Override
    public CicadaPlayer.MirrorMode getMirrorMode() {
        return CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
    }

    @Override
    public void setMirrorMode(CicadaPlayer.MirrorMode mirrorMode) {

    }

    @Override
    public void setVideoBackgroundColor(long color) {

    }

    @Override
    public int getCurrentStreamIndex(StreamType streamType) {
        if (mSystemMediaPlayer != null) {

            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
                int type = android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_UNKNOWN;
                if (streamType == StreamType.ST_TYPE_AUDIO) {
                    type = android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_AUDIO;
                } else if (streamType == StreamType.ST_TYPE_VIDEO) {
                    type = android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_VIDEO;
                } else if (streamType == StreamType.ST_TYPE_SUB) {
                    type = android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_SUBTITLE;
                }
                return mSystemMediaPlayer.getSelectedTrack(type);
            } else {
                return -1;
            }
        }
        return -1;
    }

    @Override
    public TrackInfo getCurrentStreamInfo(StreamType streamType) {
        int index = getCurrentStreamIndex(streamType);
        if (index >= 0 && mTrackInfos != null && index < mTrackInfos.length) {
            android.media.MediaPlayer.TrackInfo trackInfo = mTrackInfos[index];
            return convert(trackInfo, index);
        }
        return null;
    }

    private TrackInfo convert(android.media.MediaPlayer.TrackInfo trackInfo, int index) {
        TrackInfo cicadaTrackInfo = new TrackInfo();
        cicadaTrackInfo.index = index;

        int trackType = trackInfo.getTrackType();
        if (trackType == android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_AUDIO) {
            cicadaTrackInfo.mType = TrackInfo.Type.TYPE_AUDIO;
        } else if (trackType == android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_VIDEO) {
            cicadaTrackInfo.mType = TrackInfo.Type.TYPE_VIDEO;
        } else if (trackType == android.media.MediaPlayer.TrackInfo.MEDIA_TRACK_TYPE_SUBTITLE) {
            cicadaTrackInfo.mType = TrackInfo.Type.TYPE_SUBTITLE;
        }
        cicadaTrackInfo.description = trackInfo.getLanguage();
        return cicadaTrackInfo;
    }

    @Override
    public long getMasterClockPts() {
        return 0;
    }

    @Override
    public void setTimeout(int timeOut) {

    }

    @Override
    public void setDropBufferThreshold(int dropValue) {

    }

    @Override
    public DecoderType getDecoderType() {
        return DecoderType.DT_HARDWARE;
    }

    @Override
    public void setDecoderType(DecoderType type) {

    }

    @Override
    public float getVolume() {
        return lastVolume;
    }

    @Override
    public void setVolume(float volume) {
        if (isMute) {
            lastVolume = volume;
        } else {
            if (mSystemMediaPlayer != null) {
                mSystemMediaPlayer.setVolume(volume, volume);
            }
        }
    }

    @Override
    public void setRefer(String refer) {
        mRefer = refer;
    }

    @Override
    public void setUserAgent(String userAgent) {
        mUserAgent = userAgent;
    }

    @Override
    public boolean isLooping() {
        if (mSystemMediaPlayer != null) {
            return mSystemMediaPlayer.isLooping();
        } else {
            return false;
        }
    }

    @Override
    public void setLooping(boolean bCirclePlay) {
        if (mSystemMediaPlayer != null) {
            mSystemMediaPlayer.setLooping(bCirclePlay);
        }
    }

    @Override
    public void captureScreen() {
        //
    }

    @Override
    public int getVideoWidth() {
        if (mSystemMediaPlayer != null) {
            return mSystemMediaPlayer.getVideoWidth();
        } else {
            return 0;
        }
    }

    @Override
    public int getVideoHeight() {
        if (mSystemMediaPlayer != null) {
            return mSystemMediaPlayer.getVideoHeight();
        } else {
            return 0;
        }
    }

    @Override
    public int getVideoRotation() {
        return 0;
    }

    @Override
    public String getPropertyString(PropertyKey key) {
        return null;
    }

    @Override
    public long getPropertyInt(PropertyKey key) {
        return 0;
    }

    @Override
    public long getPropertyLong(PropertyKey key) {
        return 0;
    }

    @Override
    public float getVideoDecodeFps() {
        return 0;
    }

    @Override
    public int setOption(String key, String value) {
        return 0;
    }

    @Override
    public String getOption(String key) {
        return null;
    }

    @Override
    public float getSpeed() {
        return mSpeed;
    }

    @Override
    public void setSpeed(float speed) {
        if (mSystemMediaPlayer != null) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                mSpeed = speed;
                PlaybackParams params = new PlaybackParams();
                params.setSpeed(speed);
                mSystemMediaPlayer.setPlaybackParams(params);
            }
        }
    }

    private Map<String, String> mCustomHeaders = new HashMap<>();

    @Override
    public void addCustomHttpHeader(String httpHeader) {
        if (TextUtils.isEmpty(httpHeader) && !httpHeader.contains(":")) {
            return;
        }
        String[] header = httpHeader.split(":");
        mCustomHeaders.put(header[0], header[1]);
    }

    @Override
    public void removeAllCustomHttpHeader() {
        mCustomHeaders.clear();
    }

    @Override
    public void addExtSubtitle(String uri) {

    }

    @Override
    public int selectExtSubtitle(int index, boolean bSelect) {
        return 0;
    }

    @Override
    public void reLoad() {

    }

    @Override
    public boolean isAutoPlay() {
        return mAutoPlay;
    }

    @Override
    public void setAutoPlay(boolean bAutoPlay) {
        mAutoPlay = bAutoPlay;
    }

    @Override
    public int invokeComponent(String content) {
        return 0;
    }

    @Override
    public void setOnPreparedListener(OnPreparedListener onPreparedListener) {
        mOutOnPreparedListener = onPreparedListener;
    }

    @Override
    public void setOnLoopingStartListener(OnLoopingStartListener onLoopingStartListener) {
        mOutOnLoopingStartListener = onLoopingStartListener;
    }

    @Override
    public void setOnCompletionListener(OnCompletionListener onCompletionListener) {
        mOutOnCompletionListener = onCompletionListener;
    }

    @Override
    public void setOnFirstFrameRenderListener(OnFirstFrameRenderListener onFirstFrameRenderListener) {
        mOutOnFirstFrameRenderListener = onFirstFrameRenderListener;
    }

    @Override
    public void setOnLoadStatusListener(OnLoadStatusListener onLoadStatusListener) {
        mOutOnLoadStatusListener = onLoadStatusListener;
    }

    @Override
    public void setOnAutoPlayStartListener(OnAutoPlayStartListener onAutoPlayStartListener) {
        mOutOnAutoPlayStartListener = onAutoPlayStartListener;
    }

    @Override
    public void setOnSeekStatusListener(OnSeekStatusListener onSeekStatusListener) {
        mOutOnSeekStatusListener = onSeekStatusListener;
    }

    @Override
    public void setOnPositionUpdateListener(OnPositionUpdateListener onPositionUpdateListener) {
        mOutOnPositionUpdateListener = onPositionUpdateListener;
    }

    @Override
    public void setOnBufferPositionUpdateListener(OnBufferPositionUpdateListener onBufferPositionUpdateListener) {
        mOutOnBufferPositionUpdateListener = onBufferPositionUpdateListener;
    }

    @Override
    public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener onVideoSizeChangedListener) {
        mOutOnVideoSizeChangedListener = onVideoSizeChangedListener;
    }

    @Override
    public void setOnStatusChangedListener(OnStatusChangedListener onStatusChangedListener) {
        mOutOnStatusChangedListener = onStatusChangedListener;
    }

    @Override
    public void setOnVideoRenderedListener(OnVideoRenderedListener onVideoRenderedListener) {
        mOutOnVideoRenderedListener = onVideoRenderedListener;
    }

    @Override
    public void setOnErrorListener(OnErrorListener onErrorListener) {
        mOutOnErrorListener = onErrorListener;
    }

    @Override
    public void setOnEventListener(OnEventListener onEventListener) {
        mOutOnEventListener = onEventListener;
    }

    @Override
    public void setOnStreamInfoGetListener(OnStreamInfoGetListener onStreamInfoGetListener) {
        mOutOnStreamInfoGetListener = onStreamInfoGetListener;
    }

    @Override
    public void setOnStreamSwitchSucListener(OnStreamSwitchSucListener onStreamSwitchSucListener) {
        mOutOnStreamSwitchSucListener = onStreamSwitchSucListener;
    }

    @Override
    public void setOnCaptureScreenListener(OnCaptureScreenListener onCaptureScreenListener) {
        mOutOnCaptureScreenListener = onCaptureScreenListener;
    }

    @Override
    public void setOnSubtitleListener(OnSubtitleListener onSubtitleListener) {
        mOutOnSubtitleListener = onSubtitleListener;
    }

    @Override
    public void setOnDrmCallback(OnDRMCallback onDRMCallback) {
        mOutOnDRMCallback = onDRMCallback;
    }
}
