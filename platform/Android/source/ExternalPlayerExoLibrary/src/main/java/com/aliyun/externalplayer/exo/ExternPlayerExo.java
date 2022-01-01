package com.aliyun.externalplayer.exo;

import android.content.Context;
import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.text.TextUtils;
import android.view.Surface;
import android.view.TextureView;

import com.cicada.player.CicadaExternalPlayer;
import com.cicada.player.CicadaPlayer;
import com.cicada.player.bean.ErrorCode;
import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.Options;
import com.cicada.player.nativeclass.TrackInfo;
import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.DefaultRenderersFactory;
import com.google.android.exoplayer2.ExoPlaybackException;
import com.google.android.exoplayer2.ExoPlayerFactory;
import com.google.android.exoplayer2.Format;
import com.google.android.exoplayer2.PlaybackParameters;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.drm.DefaultDrmSessionManager;
import com.google.android.exoplayer2.drm.DrmSessionManager;
import com.google.android.exoplayer2.drm.FrameworkMediaCrypto;
import com.google.android.exoplayer2.drm.FrameworkMediaDrm;
import com.google.android.exoplayer2.drm.MediaDrmCallback;
import com.google.android.exoplayer2.drm.UnsupportedDrmException;
import com.google.android.exoplayer2.source.ExtractorMediaSource;
import com.google.android.exoplayer2.source.MediaSource;
import com.google.android.exoplayer2.source.MergingMediaSource;
import com.google.android.exoplayer2.source.SingleSampleMediaSource;
import com.google.android.exoplayer2.source.TrackGroup;
import com.google.android.exoplayer2.source.TrackGroupArray;
import com.google.android.exoplayer2.source.dash.DashMediaSource;
import com.google.android.exoplayer2.source.hls.HlsMediaSource;
import com.google.android.exoplayer2.source.smoothstreaming.SsMediaSource;
import com.google.android.exoplayer2.text.Cue;
import com.google.android.exoplayer2.text.TextOutput;
import com.google.android.exoplayer2.trackselection.DefaultTrackSelector;
import com.google.android.exoplayer2.trackselection.MappingTrackSelector;
import com.google.android.exoplayer2.trackselection.TrackSelection;
import com.google.android.exoplayer2.trackselection.TrackSelectionArray;
import com.google.android.exoplayer2.util.MimeTypes;
import com.google.android.exoplayer2.util.Util;
import com.google.android.exoplayer2.video.VideoListener;

import java.util.ArrayList;
import java.util.List;

public class ExternPlayerExo extends CicadaExternalPlayer {
    private static final String PLAYER_NAME = "ExoPlayer";
    private static final String TAG = PLAYER_NAME;

    PlayerStatus mLastPlayState = PlayerStatus.PLAYER_IDLE;
    private Context mContext = null;
    private SimpleExoPlayer mExoPlayer = null;
    private String mPlayUrl;
    private OnPreparedListener mOutOnPreparedListener;
    private OnLoopingStartListener mOutOnLoopingStartListener;
    private OnCompletionListener mOutOnCompletionListener;
    private OnFirstFrameRenderListener mOutOnFirstFrameRenderListener;
    private OnLoadStatusListener mOutOnLoadStatusListener;
    private OnAutoPlayStartListener mOutOnAutoPlayStartListener;
    private OnSeekStatusListener mOutOnSeekStatusListener;
    private OnPositionUpdateListener mOutOnPositionUpdateListener;
    private OnBufferPositionUpdateListener mOutOnBufferPositionUpdateListener;
    private OnVideoSizeChangedListener mOutOnVideoSizeChangedListener;
    private OnStatusChangedListener mOutOnStatusChangedListener;
    private OnVideoRenderedListener mOutOnVideoRenderedListener;
    private OnErrorListener mOutOnErrorListener;
    private OnEventListener mOutOnEventListener;
    private OnStreamInfoGetListener mOutOnStreamInfoGetListener;
    private OnStreamSwitchSucListener mOutOnStreamSwitchSucListener;
    private OnCaptureScreenListener mOutOnCaptureScreenListener;
    private OnSubtitleListener mOutOnSubtitleListener;
    private OnDRMCallback mOutOnDRMCallback;

    private List<ExoTrackInfo> mExoTrackInfoList = new ArrayList<>();
    private DefaultTrackSelector mTrackSelector;
    private Handler timer = null;
    private int TIMER_WHAT_HALF_SECOND = 1000;
    private ExoTrackInfo mLastSwitchTrackInfo = null;
    private boolean mMute = false;
    private float mLastVolume = 1f;
    private String mUserAgent = null;
    private ExternHttpDataSourceFactory mDataSourceFactory = null;
    private Surface mSurface = null;
    private int mTimeOut = 5 * 1000;
    private List<String> mHttpHeader = new ArrayList<>();
    private List<String> mExtSubtitleUrls = new ArrayList<>();
    private FrameworkMediaDrm mediaDrm;
    private CicadaPlayer.MirrorMode mMirrorMode = CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
    private CicadaPlayer.RotateMode mRotateMode = CicadaPlayer.RotateMode.ROTATE_0;
    private CicadaPlayer.ScaleMode mScaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
    private TextureView mTextureView;
    private int mVideoHeight;
    private int mVideoWith;

    private ExternPlayerExo(Context context, Options options) {
        mContext = context;
        Looper looper = Looper.myLooper();
        if (looper == null) {
            looper = Looper.getMainLooper();
        }

        timer = new Handler(looper) {
            @Override
            public void handleMessage(Message msg) {
                if (msg.what == TIMER_WHAT_HALF_SECOND) {

                    if (mLastPlayState.getValue() >= PlayerStatus.PLAYER_PREPARED.getValue()
                            && mLastPlayState.getValue() <= PlayerStatus.PLAYER_COMPLETION.getValue()) {
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

    }

    public ExternPlayerExo() {

    }

    private static boolean isSupportRender(MappingTrackSelector.MappedTrackInfo mappedTrackInfo, int rendererIndex) {
        TrackGroupArray trackGroupArray = mappedTrackInfo.getTrackGroups(rendererIndex);
        if (trackGroupArray.length == 0) {
            return false;
        }
        int trackType = mappedTrackInfo.getRendererType(rendererIndex);
        switch (trackType) {
            case C.TRACK_TYPE_VIDEO:
            case C.TRACK_TYPE_AUDIO:
            case C.TRACK_TYPE_TEXT:
                return true;
            default:
                return false;
        }
    }

    private static int inferPrimaryTrackType(Format format) {
        int trackType = MimeTypes.getTrackType(format.sampleMimeType);
        if (trackType != C.TRACK_TYPE_UNKNOWN) {
            return trackType;
        }
        if (MimeTypes.getVideoMediaMimeType(format.codecs) != null) {
            return C.TRACK_TYPE_VIDEO;
        }
        if (MimeTypes.getAudioMediaMimeType(format.codecs) != null) {
            return C.TRACK_TYPE_AUDIO;
        }
        if (format.width != Format.NO_VALUE || format.height != Format.NO_VALUE) {
            return C.TRACK_TYPE_VIDEO;
        }
        if (format.channelCount != Format.NO_VALUE || format.sampleRate != Format.NO_VALUE) {
            return C.TRACK_TYPE_AUDIO;
        }
        return C.TRACK_TYPE_UNKNOWN;
    }

    private void addSubtitleListener() {
        mExoPlayer.addTextOutput(new TextOutput() {
            @Override
            public void onCues(List<Cue> cues) {
                int cicadaIndex = -1;
                for (ExoTrackInfo info : mExoTrackInfoList) {
                    if (info.type == C.TRACK_TYPE_TEXT && info.selected) {
                        cicadaIndex = info.cicadaTrack.index;
                        break;
                    }
                }
                if (cicadaIndex < 0) {
                    return;
                }
                //字幕相关的接口
                if (cues == null || cues.size() == 0) {
                    //隐藏
                    if (mOutOnSubtitleListener != null) {
                        mOutOnSubtitleListener.onSubtitleHide(cicadaIndex, 0);
                    }
                } else {
                    String text = String.valueOf(cues.get(0).text);
                    if (mOutOnSubtitleListener != null) {
                        mOutOnSubtitleListener.onSubtitleShow(cicadaIndex, 0, text);
                    }
                }
            }
        });
    }

    private void updateSelectedTrack(TrackSelectionArray trackSelections) {
        for (ExoTrackInfo trackInfo : mExoTrackInfoList) {
            trackInfo.selected = false;
        }

        int selectionCount = trackSelections.length;
        for (int selectIndex = 0; selectIndex < selectionCount; selectIndex++) {
            TrackSelection trackSelection = trackSelections.get(selectIndex);
            if (trackSelection == null) {
                continue;
            }
            Format selectedFormat = trackSelection.getSelectedFormat();
            for (ExoTrackInfo trackInfo : mExoTrackInfoList) {
                if (trackInfo.exoFormat == selectedFormat) {
                    trackInfo.selected = true;
                }
            }
        }
    }

    private void sendHalfSecondTimer() {
        timer.removeMessages(TIMER_WHAT_HALF_SECOND);
        if (mLastPlayState.getValue() >= PlayerStatus.PLAYER_PREPARED.getValue()
                && mLastPlayState.getValue() <= PlayerStatus.PLAYER_COMPLETION.getValue()) {
            timer.sendEmptyMessageDelayed(TIMER_WHAT_HALF_SECOND, 500);
        }
    }

    private void notifyGetTrackInfo() {
        if (mOutOnStreamInfoGetListener != null) {
            int trackSize = mExoTrackInfoList.size();
            TrackInfo[] trackInfos = new TrackInfo[trackSize];
            for (int i = 0; i < trackSize; i++) {
                trackInfos[i] = mExoTrackInfoList.get(i).cicadaTrack;
            }
            MediaInfo info = new MediaInfo();
            info.setTrackInfos(trackInfos);
            mOutOnStreamInfoGetListener.OnStreamInfoGet(info);
        }
    }

    private void fillExoTrackInfoList() {
        mExoTrackInfoList.clear();

        MappingTrackSelector.MappedTrackInfo mappedTrackInfo = mTrackSelector.getCurrentMappedTrackInfo();
        if (mappedTrackInfo == null) {
            return;
        }

        int renderCount = mappedTrackInfo.getRendererCount();
        int cicadaTrackIndex = 0;
        for (int renderIndex = 0; renderIndex < renderCount; renderIndex++) {
            if (!isSupportRender(mappedTrackInfo, renderIndex)) {
                continue;
            }

            TrackGroupArray trackGroupArray = mappedTrackInfo.getTrackGroups(renderIndex);
            int length = trackGroupArray.length;
            for (int groupIndex = 0; groupIndex < length; groupIndex++) {
                TrackGroup trackGroup = trackGroupArray.get(groupIndex);
                int formatLength = trackGroup.length;
                for (int trackIndex = 0; trackIndex < formatLength; trackIndex++) {
                    Format format = trackGroup.getFormat(trackIndex);
                    int trackType = inferPrimaryTrackType(format);
                    TrackInfo cicadaTrack = null;
                    if (trackType == C.TRACK_TYPE_VIDEO) {
                        //视频流
                        cicadaTrack = new TrackInfo();
                        cicadaTrack.index = cicadaTrackIndex;
                        cicadaTrack.mType = TrackInfo.Type.TYPE_VIDEO;
                        cicadaTrack.videoBitrate = format.bitrate;
                        cicadaTrack.videoWidth = format.width;
                        cicadaTrack.videoHeight = format.height;
                    } else if (trackType == C.TRACK_TYPE_AUDIO) {
                        //音频流
                        cicadaTrack = new TrackInfo();
                        cicadaTrack.index = cicadaTrackIndex;
                        cicadaTrack.mType = TrackInfo.Type.TYPE_AUDIO;
                        cicadaTrack.audioLang = format.language;
                        cicadaTrack.audioChannels = format.channelCount;
                        cicadaTrack.audioSampleRate = format.sampleRate;
                        //TODO  audioSampleFormat 这个值设置
//                    cicadaTrack.audioSampleFormat = format.;
                    } else if (trackType == C.TRACK_TYPE_TEXT) {
                        //字幕流
                        cicadaTrack = new TrackInfo();
                        cicadaTrack.index = cicadaTrackIndex;
                        cicadaTrack.mType = TrackInfo.Type.TYPE_SUBTITLE;
                        cicadaTrack.subtitleLang = format.language;
                    }

                    if (cicadaTrack != null) {
                        cicadaTrackIndex++;
                        ExoTrackInfo exoTrackInfo = new ExoTrackInfo();
                        exoTrackInfo.exoFormat = format;
                        exoTrackInfo.groupIndex = groupIndex;
                        exoTrackInfo.renderIndex = renderIndex;
                        exoTrackInfo.trackIndex = trackIndex;
                        exoTrackInfo.cicadaTrack = cicadaTrack;
                        exoTrackInfo.selected = false;
                        exoTrackInfo.type = trackType;
                        mExoTrackInfoList.add(exoTrackInfo);
                    }
                }
            }
        }

    }

    private void changePlayState(PlayerStatus newState) {
        PlayerStatus oldState = mLastPlayState;
        mLastPlayState = newState;
        if (oldState != newState) {
            sendHalfSecondTimer();
            if (mOutOnStatusChangedListener != null) {
                mOutOnStatusChangedListener.onStatusChanged(oldState.getValue(), newState.getValue());
            }
        }

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
        return new ExternPlayerExo(context, options);
    }

    @Override
    public void setDataSource(String url) {
        mPlayUrl = url;
        changePlayState(PlayerStatus.PLAYER_INITIALZED);
    }

    private MediaSource buildMediaSource() {
        Uri uri = Uri.parse(mPlayUrl);
        int type = Util.inferContentType(uri);
        mDataSourceFactory = new ExternHttpDataSourceFactory();
        mDataSourceFactory.setReadTimeoutMillis(mTimeOut);
        mDataSourceFactory.setConnectTimeoutMillis(mTimeOut);
        mDataSourceFactory.setUserAgent(mUserAgent);
        for (String header : mHttpHeader) {
            mDataSourceFactory.addHttpHeaders(header);
        }

        MediaSource mediaSource = null;
        switch (type) {
            case C.TYPE_DASH:
                mediaSource = new DashMediaSource.Factory(mDataSourceFactory).createMediaSource(uri);
                break;
            case C.TYPE_SS:
                mediaSource = new SsMediaSource.Factory(mDataSourceFactory).createMediaSource(uri);
                break;
            case C.TYPE_HLS:
                mediaSource = new HlsMediaSource.Factory(mDataSourceFactory).createMediaSource(uri);
                break;
            case C.TYPE_OTHER:
                mediaSource = new ExtractorMediaSource.Factory(mDataSourceFactory).createMediaSource(uri);
                break;
        }

        MergingMediaSource mergedSource = null;

        //测试外部字幕
//        mExtSubtitleUrls.add("https://alivc-demo-vod.aliyuncs.com/b4da45beb07b4d5b81b54b1ac50fb502/subtitles/cn/6b4949a8c3950f8aa76f1fed6730e525.vtt");
        if (!mExtSubtitleUrls.isEmpty()) {

            int size = mExtSubtitleUrls.size();
            for (int i = 0; i < size; i++) {
                Format textFormat = Format.createTextSampleFormat(null, MimeTypes.TEXT_VTT, null,
                        Format.NO_VALUE, 0, "Subtitle:" + i, null, 0);
                Uri subtitleUri = Uri.parse(mExtSubtitleUrls.get(i));
                MediaSource subtitleSource = new SingleSampleMediaSource(subtitleUri, mDataSourceFactory, textFormat, C.TIME_UNSET);
                mergedSource = new MergingMediaSource(mediaSource, subtitleSource);
            }
        }

        if (mergedSource != null) {
            return mergedSource;
        }
        return mediaSource;
    }

    @Override
    public void setSurface(Surface surface) {
        if (mExoPlayer == null) {
            mSurface = surface;
        } else {
            mExoPlayer.setVideoSurface(surface);
        }
        if (surface instanceof ExternExoSurface) {
            mTextureView = ((ExternExoSurface) surface).getTextureView();
        }
    }

    @Override
    public void prepare() {
        MediaSource mMediaSource = buildMediaSource();
        if (mMediaSource == null) {
            return;
        }

        initPlayer();

        if (mExoPlayer == null) {
            return;
        }

        changePlayState(PlayerStatus.PLAYER_PREPARING);
        mExoPlayer.prepare(mMediaSource, false, true);
    }

    private void initPlayer() {
        //TODO 判断是否是WideVine的流
        boolean isWideVine = true;
        DrmSessionManager<FrameworkMediaCrypto> drmSessionManager = null;

        if (isWideVine) {
            drmSessionManager = buildDrmSessionManagerV18();
        }

        mTrackSelector = new DefaultTrackSelector();
        mExoPlayer = ExoPlayerFactory.newSimpleInstance(
                mContext, new DefaultRenderersFactory(mContext), mTrackSelector, drmSessionManager);

        mExoPlayer.setPlayWhenReady(false);
        mExoPlayer.setVideoSurface(mSurface);

        addSubtitleListener();
        Player.VideoComponent videoComponent = mExoPlayer.getVideoComponent();
        if (videoComponent != null) {
            videoComponent.addVideoListener(new VideoListener() {
                @Override
                public void onVideoSizeChanged(int width, int height, int unappliedRotationDegrees, float pixelWidthHeightRatio) {
                    mVideoWith = width;
                    mVideoHeight = height;
                    if (mTextureView instanceof ExternExoTextureView) {
                        ((ExternExoTextureView) mTextureView).setVideoSize(mVideoWith, mVideoHeight);
                        mTextureView.requestLayout();
                    }
                    if (mOutOnVideoSizeChangedListener != null) {
                        mOutOnVideoSizeChangedListener.onVideoSizeChanged(width, height);
                    }
                }

                @Override
                public void onRenderedFirstFrame() {
                    if (mExoPlayer.getPlayWhenReady()) {
                        if (mOutOnAutoPlayStartListener != null) {
                            mOutOnAutoPlayStartListener.onAutoPlayStart();
                        }
                    }

                    if (mOutOnFirstFrameRenderListener != null) {
                        mOutOnFirstFrameRenderListener.onFirstFrameRender();
                    }
                }
            });
        }

        mExoPlayer.addListener(new Player.EventListener() {
            @Override
            public void onPlayerStateChanged(boolean playWhenReady, int playbackState) {
                if (playbackState == Player.STATE_ENDED) {
                    changePlayState(PlayerStatus.PLAYER_COMPLETION);
                    if (mOutOnCompletionListener != null) {
                        mOutOnCompletionListener.onCompletion();
                    }

                } else if (playbackState == Player.STATE_BUFFERING) {

                }

            }

            @Override
            public void onLoadingChanged(boolean isLoading) {

                if (mOutOnLoadStatusListener != null) {
                    if (isLoading) {
                        mOutOnLoadStatusListener.onLoadingStart();
                    } else {
                        mOutOnLoadStatusListener.onLoadingEnd();
                    }
                }
            }

            @Override
            public void onPlayerError(ExoPlaybackException error) {
                changePlayState(PlayerStatus.PLAYER_ERROR);

                if (mOutOnErrorListener != null) {
                    if (error.type == ExoPlaybackException.TYPE_SOURCE) {
                        mOutOnErrorListener.onError(ErrorCode.ERROR_DEMUXER_OPENURL.getValue(), error.getMessage());
                    } else if (error.type == ExoPlaybackException.TYPE_RENDERER) {
                        mOutOnErrorListener.onError(ErrorCode.ERROR_CODEC_UNKNOWN.getValue(), error.getMessage());
                    } else if (error.type == ExoPlaybackException.TYPE_UNEXPECTED) {
                        mOutOnErrorListener.onError(ErrorCode.ERROR_UNKNOWN.getValue(), error.getMessage());
                    }
                }
            }

            @Override
            public void onTracksChanged(TrackGroupArray trackGroups, TrackSelectionArray trackSelections) {
                if (mExoTrackInfoList.isEmpty()) {
                    fillExoTrackInfoList();
                    notifyGetTrackInfo();
                }

                if (mLastPlayState == PlayerStatus.PLAYER_PREPARING) {
                    //播放多码率时，不会回调onPlayerStateChanged的回调
                    changePlayState(PlayerStatus.PLAYER_PREPARED);
                    //准备完成
                    if (mOutOnPreparedListener != null) {
                        mOutOnPreparedListener.onPrepared();
                    }
                }

                updateSelectedTrack(trackSelections);

                if (mOutOnStreamSwitchSucListener != null && mLastSwitchTrackInfo != null) {
                    mOutOnStreamSwitchSucListener.onStreamSwitchSuc(convertType(mLastSwitchTrackInfo.cicadaTrack.mType), mLastSwitchTrackInfo.cicadaTrack);
                }
            }

            @Override
            public void onSeekProcessed() {
                if (mOutOnSeekStatusListener != null) {
                    mOutOnSeekStatusListener.onSeekEnd(false);
                }
            }

            @Override
            public void onPositionDiscontinuity(int reason) {
                if (reason == Player.DISCONTINUITY_REASON_PERIOD_TRANSITION) {
                    //循环播放
                    if (mOutOnLoopingStartListener != null) {
                        mOutOnLoopingStartListener.onLoopingStart();
                    }
                }
            }
        });

    }

    @Override
    public void start() {
        if (mExoPlayer == null) {
            return;
        }

        mExoPlayer.setPlayWhenReady(true);

        changePlayState(PlayerStatus.PLAYER_PLAYING);
    }

    @Override
    public void pause() {
        if (mExoPlayer == null) {
            return;
        }

        mExoPlayer.setPlayWhenReady(false);
        changePlayState(PlayerStatus.PLAYER_PAUSED);
    }

    @Override
    public void stop() {
        if (mExoPlayer == null) {
            return;
        }
        mExoPlayer.stop(true);
        changePlayState(PlayerStatus.PLAYER_STOPPED);
    }

    @Override
    public void release() {
        if (mExoPlayer == null) {
            return;
        }
        changePlayState(PlayerStatus.PLAYER_IDLE);
        releaseMediaDrm();
        mExoPlayer.release();

    }

    @Override
    public StreamType switchStream(int index) {
        ExoTrackInfo targetInfo = null;
        for (ExoTrackInfo info : mExoTrackInfoList) {
            if (info.cicadaTrack.index == index) {
                targetInfo = info;
                break;
            }
        }

        if (targetInfo == null) {
            return StreamType.ST_TYPE_UNKNOWN;
        }

        StreamType streamType = convertType(targetInfo.cicadaTrack.mType);
        boolean success = selectTrack(targetInfo);
        return streamType;
    }

    private boolean selectTrack(ExoTrackInfo exoTrackInfo) {
        if (mTrackSelector == null) {
            return false;
        }
        MappingTrackSelector.MappedTrackInfo mappedTrackInfo = mTrackSelector.getCurrentMappedTrackInfo();
        if (mappedTrackInfo == null) {
            return false;
        }
        DefaultTrackSelector.Parameters parameters = mTrackSelector.getParameters();
        if (parameters == null) {
            return false;
        }

        mLastSwitchTrackInfo = exoTrackInfo;
        if (exoTrackInfo.cicadaTrack.mType == TrackInfo.Type.TYPE_SUBTITLE) {
            changeSubtitleSelection(true, exoTrackInfo);
        }

        DefaultTrackSelector.ParametersBuilder builder = parameters.buildUpon();
        builder.setSelectionOverride(
                exoTrackInfo.renderIndex,
                mappedTrackInfo.getTrackGroups(exoTrackInfo.renderIndex),
                new DefaultTrackSelector.SelectionOverride(exoTrackInfo.groupIndex, exoTrackInfo.trackIndex));

        mTrackSelector.setParameters(builder);

        return true;
    }

    private StreamType convertType(TrackInfo.Type type) {
        if (type == TrackInfo.Type.TYPE_AUDIO) {
            return StreamType.ST_TYPE_AUDIO;
        } else if (type == TrackInfo.Type.TYPE_VIDEO) {
            return StreamType.ST_TYPE_VIDEO;
        } else if (type == TrackInfo.Type.TYPE_SUBTITLE) {
            return StreamType.ST_TYPE_SUB;
        }
        return StreamType.ST_TYPE_UNKNOWN;
    }

    @Override
    public void seekTo(long seekPos, boolean accurate) {
        if (mExoPlayer == null) {
            return;
        }

        if (mOutOnSeekStatusListener != null) {
            mOutOnSeekStatusListener.onSeekStart(false);
        }

        mExoPlayer.seekTo(seekPos);
    }

    @Override
    public PlayerStatus getPlayerStatus() {
        return mLastPlayState;
    }

    @Override
    public long getDuration() {
        if (mExoPlayer == null) {
            return 0;
        }
        long duration = mExoPlayer.getDuration();
        return duration;
    }

    @Override
    public long getPlayingPosition() {
        if (mExoPlayer == null) {
            return 0;
        }
        return mExoPlayer.getCurrentPosition();
    }

    @Override
    public long getBufferPosition() {
        if (mExoPlayer == null) {
            return 0;
        }
        return mExoPlayer.getBufferedPosition();
    }

    @Override
    public void mute(boolean mute) {
        if (mExoPlayer == null) {
            return;
        }

        mMute = mute;
        if (mute) {
            mLastVolume = getVolume();
            setVolume(0f);
        } else {
            setVolume(mLastVolume);
        }
    }

    @Override
    public boolean isMute() {
        return mMute;
    }

    @Override
    public float getVideoRenderFps() {
        //TODO
        return 0;
    }

    @Override
    public void enterBackGround(boolean back) {
        //TODO
    }

    @Override
    public CicadaPlayer.ScaleMode getScaleMode() {
        if (mExoPlayer == null) {
            return CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
        }

        int mode = mExoPlayer.getVideoScalingMode();
        CicadaPlayer.ScaleMode scaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
        if (mode == C.VIDEO_SCALING_MODE_SCALE_TO_FIT) {
            scaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT;
        } else if (mode == C.VIDEO_SCALING_MODE_SCALE_TO_FIT_WITH_CROPPING) {
            scaleMode = CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL;
        } else if (mode == C.VIDEO_SCALING_MODE_DEFAULT) {
            scaleMode = CicadaPlayer.ScaleMode.SCALE_TO_FILL;
        }

        return scaleMode;
    }

    @Override
    public void setScaleMode(CicadaPlayer.ScaleMode scaleMode) {
        if (mExoPlayer == null) {
            return;
        }

        if (mTextureView == null) {
            return;
        }

        ((ExternExoTextureView) mTextureView).setScaleType(scaleMode);

        this.mScaleMode = scaleMode;

        mTextureView.requestLayout();
    }

    @Override
    public CicadaPlayer.RotateMode getRotateMode() {
        if (mExoPlayer == null) {
            return CicadaPlayer.RotateMode.ROTATE_0;
        }
        return this.mRotateMode;
    }

    @Override
    public void setRotateMode(CicadaPlayer.RotateMode rotateMode) {
        if (mExoPlayer == null) {
            return;
        }

        if (mTextureView == null) {
            return;
        }

        if (rotateMode == CicadaPlayer.RotateMode.ROTATE_0) {
            mTextureView.setRotation(0f);
        } else if (rotateMode == CicadaPlayer.RotateMode.ROTATE_90) {
            mTextureView.setRotation(90f);
        } else if (rotateMode == CicadaPlayer.RotateMode.ROTATE_180) {
            mTextureView.setRotation(180f);
        } else {
            mTextureView.setRotation(270f);
        }
        mTextureView.requestLayout();

        this.mRotateMode = rotateMode;
    }

    @Override
    public CicadaPlayer.MirrorMode getMirrorMode() {
        if (mExoPlayer == null) {
            return CicadaPlayer.MirrorMode.MIRROR_MODE_NONE;
        }
        return mMirrorMode;
    }

    @Override
    public void setMirrorMode(CicadaPlayer.MirrorMode mirrorMode) {
        if (mExoPlayer == null) {
            return;
        }
        if (mTextureView == null) {
            return;
        }
        if (mirrorMode == CicadaPlayer.MirrorMode.MIRROR_MODE_NONE) {
            mTextureView.setScaleX(1f);
            mTextureView.setScaleY(1f);
        } else if (mirrorMode == CicadaPlayer.MirrorMode.MIRROR_MODE_VERTICAL) {
            mTextureView.setScaleX(1f);
            mTextureView.setScaleY(-1f);
        } else {
            mTextureView.setScaleX(-1f);
            mTextureView.setScaleY(1f);
        }
        mTextureView.invalidate();
        this.mMirrorMode = mirrorMode;
    }

    @Override
    public void setVideoBackgroundColor(long color) {
        if (mExoPlayer == null) {
            return;
        }
        //TODO
    }

    @Override
    public int getCurrentStreamIndex(StreamType streamType) {
        TrackInfo cicadaTrackInfo = getCurrentStreamInfo(streamType);
        if (cicadaTrackInfo != null) {
            return cicadaTrackInfo.index;
        }
        return -1;
    }

    @Override
    public TrackInfo getCurrentStreamInfo(StreamType streamType) {
        for (ExoTrackInfo info : mExoTrackInfoList) {
            if (info.selected) {
                if (streamType == StreamType.ST_TYPE_VIDEO && info.type == C.TRACK_TYPE_VIDEO) {
                    return info.cicadaTrack;
                } else if (streamType == StreamType.ST_TYPE_AUDIO && info.type == C.TRACK_TYPE_AUDIO) {
                    return info.cicadaTrack;
                } else if (streamType == StreamType.ST_TYPE_SUB && info.type == C.TRACK_TYPE_TEXT) {
                    return info.cicadaTrack;
                }
            }
        }
        return null;
    }

    @Override
    public long getMasterClockPts() {
        return 0;
    }

    @Override
    public void setTimeout(int timeOut) {
        mTimeOut = timeOut;
        if (mDataSourceFactory != null) {
            mDataSourceFactory.setConnectTimeoutMillis(timeOut);
            mDataSourceFactory.setReadTimeoutMillis(timeOut);
        }
    }

    @Override
    public void setDropBufferThreshold(int dropValue) {
        //TODO
    }

    @Override
    public DecoderType getDecoderType() {
        return null;
    }

    @Override
    public void setDecoderType(DecoderType type) {

    }

    @Override
    public float getVolume() {
        if (mExoPlayer == null) {
            return 0;
        }
        return mExoPlayer.getVolume();
    }

    @Override
    public void setVolume(float volume) {
        if (mExoPlayer == null) {
            return;
        }

        mLastVolume = volume;

        if (!mMute) {
            mExoPlayer.setVolume(volume);
        }
    }

    @Override
    public void setRefer(String refer) {

    }

    @Override
    public void setUserAgent(String userAgent) {
        mUserAgent = userAgent;
        if (mDataSourceFactory != null) {
            mDataSourceFactory.setUserAgent(userAgent);
        }
    }

    @Override
    public boolean isLooping() {
        if (mExoPlayer == null) {
            return false;
        }

        return mExoPlayer.getRepeatMode() == Player.REPEAT_MODE_ALL;
    }

    @Override
    public void setLooping(boolean bCirclePlay) {
        if (mExoPlayer == null) {
            return;
        }
        mExoPlayer.setRepeatMode(bCirclePlay ? Player.REPEAT_MODE_ALL : Player.REPEAT_MODE_ONE);
    }

    @Override
    public void captureScreen() {
        if (mExoPlayer == null) {
            return;
        }

        //TODO
    }

    @Override
    public int getVideoWidth() {
        if (mExoPlayer == null || mExoPlayer.getVideoFormat() == null) {
            return 0;
        }
        return mExoPlayer.getVideoFormat().width;
    }

    @Override
    public int getVideoHeight() {
        if (mExoPlayer == null || mExoPlayer.getVideoFormat() == null) {
            return 0;
        }
        return mExoPlayer.getVideoFormat().height;
    }

    @Override
    public int getVideoRotation() {
        if (mExoPlayer == null || mExoPlayer.getVideoFormat() == null) {
            return 0;
        }
        return mExoPlayer.getVideoFormat().rotationDegrees;
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
        if (mExoPlayer == null) {
            return 1f;
        }
        PlaybackParameters playbackParameters = mExoPlayer.getPlaybackParameters();
        if (playbackParameters == null) {
            return 1f;
        }
        return playbackParameters.speed;
    }

    @Override
    public void setSpeed(float speed) {
        if (mExoPlayer == null) {
            return;
        }
        PlaybackParameters playbackParameters = new PlaybackParameters(speed, 1);
        mExoPlayer.setPlaybackParameters(playbackParameters);
    }

    @Override
    public void addCustomHttpHeader(String httpHeader) {
        if (!TextUtils.isEmpty(httpHeader)) {
            mHttpHeader.add(httpHeader);
        }

        if (mDataSourceFactory != null) {
            mDataSourceFactory.addHttpHeaders(httpHeader);
        }
    }

    @Override
    public void removeAllCustomHttpHeader() {
        mHttpHeader.clear();

        if (mDataSourceFactory != null) {
            mDataSourceFactory.clearHttpHeaders();
        }
    }

    @Override
    public void addExtSubtitle(String uri) {
        if (!TextUtils.isEmpty(uri)) {
            mExtSubtitleUrls.add(uri);
        }
    }

    @Override
    public int selectExtSubtitle(int index, boolean bSelect) {
        ExoTrackInfo targetInfo = null;
        for (ExoTrackInfo info : mExoTrackInfoList) {
            if (info.cicadaTrack.index == index) {
                targetInfo = info;
            }
        }

        if (targetInfo == null) {
            return -1;
        }

        if (!bSelect) {
            int cicadaIndex = -1;
            for (ExoTrackInfo info : mExoTrackInfoList) {
                if (info.type == C.TRACK_TYPE_TEXT && info.selected) {
                    cicadaIndex = info.cicadaTrack.index;
                    break;
                }
            }

            if (cicadaIndex == targetInfo.cicadaTrack.index) {
                if (mOutOnSubtitleListener != null) {
                    mOutOnSubtitleListener.onSubtitleHide(targetInfo.cicadaTrack.index, 0);
                }
            }
        }

        boolean b = changeSubtitleSelection(bSelect, targetInfo);
        return b ? 0 : -1;
    }

    private boolean changeSubtitleSelection(boolean bSelect, ExoTrackInfo targetInfo) {
        TrackSelectionArray trackSelections = mExoPlayer.getCurrentTrackSelections();
        int selectionCount = trackSelections.length;
        for (int selectIndex = 0; selectIndex < selectionCount; selectIndex++) {
            TrackSelection trackSelection = trackSelections.get(selectIndex);
            if (trackSelection == null) {
                continue;
            }
            Format selectedFormat = trackSelection.getSelectedFormat();
            if (targetInfo.exoFormat == selectedFormat) {
                if (bSelect) {
                    trackSelection.enable();
                    targetInfo.selected = true;
                } else {
                    trackSelection.disable();
                    targetInfo.selected = false;
                }
                return true;
            }
        }
        return false;
    }

    @Override
    public void reLoad() {
        if (mExoPlayer == null) {
            return;
        }

        mExoPlayer.retry();
    }

    @Override
    public boolean isAutoPlay() {
        if (mExoPlayer == null) {
            return false;
        }
        return mExoPlayer.getPlayWhenReady();
    }

    @Override
    public void setAutoPlay(boolean bAutoPlay) {
        if (mExoPlayer == null) {
            return;
        }

        mExoPlayer.setPlayWhenReady(bAutoPlay);
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

    private DefaultDrmSessionManager<FrameworkMediaCrypto> buildDrmSessionManagerV18() {
        MediaDrmCallback drmCallback =
                new WideVineDrmCallback(new OnDRMCallback() {

                    @Override
                    public byte[] onRequestProvision(String provisionUrl, byte[] data) {
                        if (mOutOnDRMCallback != null) {
                            return mOutOnDRMCallback.onRequestProvision(provisionUrl, data);
                        }
                        return null;
                    }

                    @Override
                    public byte[] onRequestKey(String licenseUrl, byte[] data) {
                        if (mOutOnDRMCallback != null) {
                            return mOutOnDRMCallback.onRequestKey(licenseUrl, data);
                        }
                        return null;
                    }
                });

        releaseMediaDrm();
        try {
            mediaDrm = FrameworkMediaDrm.newInstance(C.WIDEVINE_UUID);
        } catch (UnsupportedDrmException e) {
            e.printStackTrace();
            return null;
        }
        return new DefaultDrmSessionManager<>(C.WIDEVINE_UUID, mediaDrm, drmCallback, null, false);
    }

    private void releaseMediaDrm() {
        if (mediaDrm != null) {
            mediaDrm.release();
            mediaDrm = null;
        }
    }

    private static class ExoTrackInfo {
        Format exoFormat;
        int renderIndex;
        int groupIndex;
        int trackIndex;
        TrackInfo cicadaTrack;
        boolean selected = false;
        int type;
    }

}
