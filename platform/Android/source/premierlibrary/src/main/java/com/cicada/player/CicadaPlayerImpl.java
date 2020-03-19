package com.cicada.player;


import android.content.Context;
import android.graphics.Bitmap;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.cicada.player.bean.ErrorInfo;
import com.cicada.player.bean.InfoBean;
import com.cicada.player.nativeclass.CacheConfig;
import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.NativePlayerBase;
import com.cicada.player.nativeclass.PlayerConfig;
import com.cicada.player.nativeclass.TrackInfo;

import java.lang.ref.WeakReference;

/*abstract*/ class CicadaPlayerImpl implements CicadaPlayer {

    private static final String TAG = CicadaPlayerImpl.class.getSimpleName();

    protected Context mContext = null;
    protected String mTraceID = null;
    private NativePlayerBase mCorePlayer = null;
    private MediaInfo mOutMediaInfo = null;

    private OnPreparedListener mOutOnPreparedListener = null;
    private OnPreparedListener mInnerOnPreparedListener = new InnerPrepareListener(this);

    private static class InnerPrepareListener implements OnPreparedListener {

        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerPrepareListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onPrepared() {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onPrepared();
            }
        }
    }

    private void onPrepared() {
        if (mOutOnPreparedListener != null) {
            mOutOnPreparedListener.onPrepared();
        }
    }

    private OnInfoListener mOutOnInfoListener = null;
    private OnInfoListener mInnerOnInfoListener = new InnerInfoListener(this);

    private static class InnerInfoListener implements OnInfoListener {

        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerInfoListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onInfo(InfoBean infoBean) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onInfo(infoBean);
            }
        }
    }

    private void onInfo(InfoBean infoBean) {
        if (mOutOnInfoListener != null) {
            mOutOnInfoListener.onInfo(infoBean);
        }
    }

    private OnErrorListener mOutOnErrorListener = null;
    private OnErrorListener mInnerOnErrorListener = new InnerErrorListener(this);

    private static class InnerErrorListener implements OnErrorListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerErrorListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onError(ErrorInfo errorInfo) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onError(errorInfo);
            }
        }
    }

    private void onError(ErrorInfo errorInfo) {
        if (mOutOnErrorListener != null) {
            mOutOnErrorListener.onError(errorInfo);
        }
    }

    private OnCompletionListener mOutOnCompletionListener = null;
    private OnCompletionListener mInnerOnCompletionListener = new InnerCompletionListener(this);

    private static class InnerCompletionListener implements OnCompletionListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerCompletionListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }


        @Override
        public void onCompletion() {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onCompletion();
            }
        }
    }

    private void onCompletion() {
        if (mOutOnCompletionListener != null) {
            mOutOnCompletionListener.onCompletion();
        }
    }

    private OnRenderingStartListener mOutOnRenderingStartListener = null;
    private OnRenderingStartListener mInnerOnFirstFrameShowListener = new InnerRenderListener(this);

    private static class InnerRenderListener implements OnRenderingStartListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerRenderListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onRenderingStart() {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onVideoRenderingStart();
            }
        }
    }

    private void onVideoRenderingStart() {
        if (mOutOnRenderingStartListener != null) {
            mOutOnRenderingStartListener.onRenderingStart();
        }
    }

    private OnVideoSizeChangedListener mOutOnVideoSizeChangedListener = null;
    private OnVideoSizeChangedListener mInnerOnVideoSizeChangedListener = new InnerVideoSizeChangedListener(this);

    private static class InnerVideoSizeChangedListener implements OnVideoSizeChangedListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerVideoSizeChangedListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onVideoSizeChanged(int width, int height) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onVideoSizeChanged(width, height);
            }
        }
    }

    private void onVideoSizeChanged(int width, int height) {
        if (mOutOnVideoSizeChangedListener != null) {
            mOutOnVideoSizeChangedListener.onVideoSizeChanged(width, height);
        }
    }

    private OnTrackReadyListener mOutOnTrackReadyListener = null;
    private OnTrackReadyListener mInnerOnTrackReadyListener = new InnerTrackReadyListener(this);

    private static class InnerTrackReadyListener implements OnTrackReadyListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerTrackReadyListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onTrackReady(MediaInfo mediaInfo) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onTrackReady(mediaInfo);
            }
        }
    }

    private void onTrackReady(MediaInfo mediaInfo) {
        mOutMediaInfo = mediaInfo;

        if (mOutOnTrackReadyListener != null) {
            mOutOnTrackReadyListener.onTrackReady(mediaInfo);
        }

    }


    private OnLoadingStatusListener mOutOnLoadingStatusListener = null;
    private OnLoadingStatusListener mInnerOnLoadingStatusListener = new InnerLoadingStatusListener(this);

    private static class InnerLoadingStatusListener implements OnLoadingStatusListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerLoadingStatusListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onLoadingBegin() {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onLoadingBegin();
            }
        }

        @Override
        public void onLoadingProgress(int percent, float netSpeed) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onLoadingProgress(percent, netSpeed);
            }
        }

        @Override
        public void onLoadingEnd() {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onLoadingEnd();
            }
        }
    }

    private void onLoadingBegin() {
        if (mOutOnLoadingStatusListener != null) {
            mOutOnLoadingStatusListener.onLoadingBegin();
        }
    }

    private void onLoadingProgress(int percent, float speed) {
        if (mOutOnLoadingStatusListener != null) {
            mOutOnLoadingStatusListener.onLoadingProgress((int) percent, speed);
        }
    }

    private void onLoadingEnd() {
        if (mOutOnLoadingStatusListener != null) {
            mOutOnLoadingStatusListener.onLoadingEnd();
        }
    }

    private OnSeekCompleteListener mOutOnSeekEndListener = null;
    private OnSeekCompleteListener mInnerOnSeekEndListener = new InnerSeekEndListener(this);

    private static class InnerSeekEndListener implements OnSeekCompleteListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerSeekEndListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onSeekComplete() {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onSeekComplete();
            }
        }
    }

    private void onSeekComplete() {
        if (mOutOnSeekEndListener != null) {
            mOutOnSeekEndListener.onSeekComplete();
        }
    }

    private OnSubtitleDisplayListener mOutOnSubtitleDisplayListener = null;
    private OnSubtitleDisplayListener mInnerOnSubtitleDisplayListener = new InnerSubtitleDisplayListener(this);

    private static class InnerSubtitleDisplayListener implements OnSubtitleDisplayListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerSubtitleDisplayListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onSubtitleShow(int trackIndex, long id, String data) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onSubtitleShow(trackIndex, id, data);
            }
        }

        @Override
        public void onSubtitleHide(int trackIndex, long id) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onSubtitleHide(trackIndex, id);
            }
        }

        @Override
        public void onSubtitleExtAdded(int id, String url) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onSubtitleExtAdded(id,url);
            }
        }
    }

    private void onSubtitleShow(int trackIndex, long id, String data) {
        if (mOutOnSubtitleDisplayListener != null) {
            mOutOnSubtitleDisplayListener.onSubtitleShow(trackIndex, id, data);
        }
    }

    private void onSubtitleHide(int trackIndex, long id) {
        if (mOutOnSubtitleDisplayListener != null) {
            mOutOnSubtitleDisplayListener.onSubtitleHide(trackIndex, id);
        }
    }

    private void onSubtitleExtAdded(int id, String url) {
        if (mOutOnSubtitleDisplayListener != null) {
            mOutOnSubtitleDisplayListener.onSubtitleExtAdded(id, url);
        }
    }


    private OnTrackChangedListener mOutOnTrackChangedListener = null;
    private OnTrackChangedListener mInnerOnTrackChangedListener = new InnerTrackChangedListener(this);

    private static class InnerTrackChangedListener implements OnTrackChangedListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerTrackChangedListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onChangedSuccess(TrackInfo trackInfo) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onChangedSuccess(trackInfo);
            }
        }

        @Override
        public void onChangedFail(TrackInfo trackInfo, ErrorInfo errorInfo) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onChangedFail(trackInfo, errorInfo);
            }
        }
    }

    private void onChangedSuccess(TrackInfo trackInfo) {
        if (mOutOnTrackChangedListener != null) {
            mOutOnTrackChangedListener.onChangedSuccess(trackInfo);
        }
    }

    private void onChangedFail(TrackInfo trackInfo, ErrorInfo errorInfo) {
        if (mOutOnTrackChangedListener != null) {
            mOutOnTrackChangedListener.onChangedFail(trackInfo, errorInfo);
        }
    }

//    private OnThumbnailListener mOutOnThumbnailListener = null;
//    private OnThumbnailListener mInnerOnThumbnailGetListener = new OnThumbnailListener() {
//        @Override
//        public void onGetSuccess(long position, ThumbnailBitmapInfo result) {
//            if (mOutOnThumbnailListener != null) {
//                mOutOnThumbnailListener.onGetSuccess(position, result);
//            }
//        }
//
//        @Override
//        public void onGetFail(long position, ErrorInfo errorInfo) {
//            if (mOutOnThumbnailListener != null) {
//                mOutOnThumbnailListener.onGetFail(position, errorInfo);
//            }
//        }
//
//    };

    private OnSnapShotListener mOutOnSnapShotListener = null;
    private OnSnapShotListener mInnerOnSnapShotListener = new InnerSnapShotListener(this);

    private static class InnerSnapShotListener implements OnSnapShotListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerSnapShotListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onSnapShot(Bitmap bm, int with, int height) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onSnapShot(bm, with, height);
            }
        }
    }

    private void onSnapShot(Bitmap bm, int with, int height) {
        if (mOutOnSnapShotListener != null) {
            mOutOnSnapShotListener.onSnapShot(bm, with, height);
        }
    }


    private OnStateChangedListener mOutOnStatusChangedListener = null;
    private OnStateChangedListener mInnerOnStatusChangedListener = new InnerStatusChangedListener(this);

    private static class InnerStatusChangedListener implements OnStateChangedListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerStatusChangedListener(CicadaPlayerImpl cicadaPlayerImpl) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(cicadaPlayerImpl);
        }

        @Override
        public void onStateChanged(int newState) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onStateChanged(newState);
            }
        }
    }

    private void onStateChanged(int newState) {
        if (mOutOnStatusChangedListener != null) {
            mOutOnStatusChangedListener.onStateChanged(newState);
        }
    }

    private OnVideoRenderedListener mOutOnVideoRenderedListener = null;
    private OnVideoRenderedListener mInnerOnVideoRenderedListener = new InnerVideoRenderedListener(this);

    private static class InnerVideoRenderedListener implements OnVideoRenderedListener {
        private WeakReference<CicadaPlayerImpl> cicadaPlayerImplWR;

        InnerVideoRenderedListener(CicadaPlayerImpl avpBase) {
            cicadaPlayerImplWR = new WeakReference<CicadaPlayerImpl>(avpBase);
        }

        @Override
        public void onVideoRendered(long timeMs, long pts) {
            CicadaPlayerImpl cicadaPlayerImpl = cicadaPlayerImplWR.get();
            if (cicadaPlayerImpl != null) {
                cicadaPlayerImpl.onVideoRendered(timeMs, pts);
            }
        }
    }

    private void onVideoRendered(long timeMs, long pts){
        if(mOutOnVideoRenderedListener != null){
            mOutOnVideoRenderedListener.onVideoRendered(timeMs ,pts);
        }
    };

    public CicadaPlayerImpl(Context context, String traceID) {
        mContext = context;
        mTraceID = traceID;
        mCorePlayer = new NativePlayerBase(context);
        mCorePlayer.setTraceId(mTraceID);

        bindListeners();
    }

    protected NativePlayerBase getCorePlayer() {
        return mCorePlayer;
    }

    private void bindListeners() {
        mCorePlayer.setOnErrorListener(mInnerOnErrorListener);
        mCorePlayer.setOnPreparedListener(mInnerOnPreparedListener);
        mCorePlayer.setOnInfoListener(mInnerOnInfoListener);
        mCorePlayer.setOnCompletionListener(mInnerOnCompletionListener);
        mCorePlayer.setOnRenderingStartListener(mInnerOnFirstFrameShowListener);
        mCorePlayer.setOnLoadingStatusListener(mInnerOnLoadingStatusListener);
        mCorePlayer.setOnSeekCompleteListener(mInnerOnSeekEndListener);
        mCorePlayer.setOnStateChangedListener(mInnerOnStatusChangedListener);
        mCorePlayer.setOnSubtitleDisplayListener(mInnerOnSubtitleDisplayListener);
        mCorePlayer.setOnVideoSizeChangedListener(mInnerOnVideoSizeChangedListener);
        mCorePlayer.setOnTrackInfoGetListener(mInnerOnTrackReadyListener);
        mCorePlayer.setOnTrackSelectRetListener(mInnerOnTrackChangedListener);
        mCorePlayer.setOnSnapShotListener(mInnerOnSnapShotListener);
//        mCorePlayer.setOnPcmDataListener(mInnerOnPcmDataListener);
    }

    @Override
    public MediaInfo getMediaInfo() {
        return mOutMediaInfo;
    }

    @Override
    public void setCacheConfig(CacheConfig cacheConfig) {
        if (cacheConfig == null) {
            cacheConfig = new CacheConfig();
            cacheConfig.mEnable = false;

        }

        mCorePlayer.setCacheConfig(cacheConfig);
    }

    @Override
    public void setSpeed(float speed) {
        mCorePlayer.setSpeed(speed);
    }

    @Override
    public float getSpeed() {
        return mCorePlayer.getSpeed();
    }


    @Override
    public void snapshot() {
        mCorePlayer.snapShot();
    }

    @Override
    public void setSurface(Surface surface) {
        mCorePlayer.setSurface(surface);
    }

    @Override
    public void start() {
        startInner();
    }


    private void startInner() {
        mCorePlayer.start();
    }

    @Override
    public void pause() {
        pauseInner();
    }

    private void pauseInner() {

        mCorePlayer.pause();
    }


    @Override
    public void stop() {
        stopInner();
    }

    private void stopInner() {
        mCorePlayer.stop();

    }


    @Override
    public void reset() {
        //TODO ?
//        mCorePlayer.reset();
    }

    @Override
    public void release() {
        mCorePlayer.release();
        clearListeners();
    }

    private void clearListeners() {
        mOutOnCompletionListener = null;
        mOutOnErrorListener = null;
        mOutOnLoadingStatusListener = null;
        mOutOnInfoListener = null;
//        mOutOnPcmDataListener = null;
        mOutOnPreparedListener = null;
        mOutOnRenderingStartListener = null;
        mOutOnSeekEndListener = null;
        mOutOnStatusChangedListener = null;
        mOutOnSubtitleDisplayListener = null;
//        mOutOnThumbnailListener = null;
        mOutOnTrackChangedListener = null;
        mOutOnTrackReadyListener = null;
        mOutOnVideoSizeChangedListener = null;
    }


    @Override
    public void setMirrorMode(MirrorMode mirrorMode) {
        mCorePlayer.setMirrorMode(mirrorMode);
    }

    @Override
    public MirrorMode getMirrorMode() {
        return mCorePlayer.getMirrorMode();
    }


    @Override
    public void setRotateMode(RotateMode rotateMode) {
        mCorePlayer.setRotateMode(rotateMode);
    }

    @Override
    public RotateMode getRotateMode() {

        return mCorePlayer.getRotateMode();
    }


    @Override
    public void setScaleMode(ScaleMode scaleMode) {
        mCorePlayer.setScaleMode(scaleMode);
    }

    @Override
    public ScaleMode getScaleMode() {
        return mCorePlayer.getScaleMode();
    }

//    @Override
//    public void setPlayerType(PlayerType playerType) {
//        //TODO
//    }
//
//    @Override
//    public PlayerType getPlayerType() {
//        //TODO
//        return PlayerType.AliyunPlayer;
//    }
//
//    @Override
//    public void setDecodeType(DecodeType decodeType) {
//        mCorePlayer.enableHardwareDecoder(decodeType == DecodeType.Auto);
//    }
//
//    @Override
//    public DecodeType getDecodeType() {
//        //TODO
//        return DecodeType.Soft;
//    }

    /**
     * 获取当前播放器的音量
     *
     * @return 范围是[0, 1]
     */
    @Override
    public float getVolume() {
        return mCorePlayer.getVolume();
    }


    @Override
    public void setConfig(PlayerConfig config) {
        mCorePlayer.setConfig(config);
    }

    @Override
    public PlayerConfig getConfig() {
        return mCorePlayer.getConfig();
    }

    /**
     * 设置循环播放。
     *
     * @param on true：打开循环播放。
     */
    @Override
    public void setLoop(boolean on) {
        mCorePlayer.setLoop(on);
    }

    @Override
    public boolean isLoop() {
        return mCorePlayer.isLoop();
    }


    /**
     * 设置静音
     *
     * @param on true:静音
     */
    @Override
    public void setMute(boolean on) {
        mCorePlayer.setMute(on);
    }
//
//    /**
//     * 设置auth源
//     *
//     * @param auth auth源
//     */
//    @Override
//    public void setDataSource(VidAuth auth) {
//        mCorePlayer.setDataSource(auth);
//    }
//
//    /**
//     * 设置sts源
//     *
//     * @param sts sts源
//     */
//    @Override
//    public void setDataSource(final VidSts sts) {
//        mCorePlayer.setDataSource(sts);
//    }
//
//    /**
//     * 设置mps源
//     *
//     * @param mps mps源
//     */
//    @Override
//    public void setDataSource(VidMps mps) {
//        mCorePlayer.setDataSource(mps);
//    }
//
//    /**
//     * 设置本地、网络播放源
//     *
//     * @param urlSource 本地、网络播放源
//     */
//    @Override
//    public void setDataSource(UrlSource urlSource) {
//        mCorePlayer.setDataSource(urlSource);
//    }


    @Override
    public void selectTrack(int trackInfoIndex) {
        mCorePlayer.selectTrack(trackInfoIndex);
    }

    @Override
    @Deprecated
    public TrackInfo currentTrack(int type) {
        return mCorePlayer.getCurrentTrackInfo(type);
    }

    @Override
    public TrackInfo currentTrack(TrackInfo.Type type) {
        return mCorePlayer.getCurrentTrackInfo(type.ordinal());
    }

    /**
     * 异步准备视频
     */
    @Override
    public void prepare() {
        mCorePlayer.prepare();
    }

    @Override
    public void seekTo(long positionMs) {
        seekTo(positionMs, SeekMode.Inaccurate);
    }

    @Override
    public void seekTo(long positionMs, SeekMode mode) {
        mCorePlayer.seekTo(positionMs, mode.getValue());
    }

    @Override
    public void setMaxAccurateSeekDelta(int delta){
        mCorePlayer.setMaxAccurateSeekDelta(delta);
    }


//    @Override
//    public int getPlayerStatus() {
//        return mCorePlayer.getStatus();
//    }

    @Override
    public long getDuration() {
        return mCorePlayer.getDuration();
    }

    @Override
    public int getVideoWidth() {
        return mCorePlayer.getVideoWidth();
    }

    @Override
    public int getVideoHeight() {
        return mCorePlayer.getVideoHeight();
    }

    /**
     * 获取视频源的旋转角度
     *
     * @return 角度。
     */
    @Override
    public int getVideoRotation() {
        return (int) mCorePlayer.getVideoRotation();
    }


    @Override
    public void setAutoPlay(boolean auto) {
        mCorePlayer.setAutoPlay(auto);
    }

    @Override
    public boolean isAutoPlay() {
        return mCorePlayer.isAutoPlay();
    }

    @Override
    public void setVolume(float volume) {
        mCorePlayer.setVolume(volume);
    }

//    @Override
//    public Map<String, String> getAllDebugInfo() {
//        //TODO 这个是需要实现的！！！
//        return null;
//    }


//    @Override
//    public long getBufferedPosition() {
//        return mCorePlayer.getBufferedPosition();
//    }

    @Override
    public boolean isMute() {
        return mCorePlayer.isMuted();
    }

//    @Override
//    public void getThumbnail(long positionMs) {
//
//        if (mThumbnailHelper == null) {
//            if (mInnerOnThumbnailGetListener != null) {
//                ErrorInfo errorInfo = new ErrorInfo();
//                errorInfo.setCode(ErrorCode.ERROR_UNKNOWN);
//                errorInfo.setMsg("not set thumbnail url");
//                mInnerOnThumbnailGetListener.onGetFail(positionMs, errorInfo);
//            }
//            return;
//        }
//
//        mThumbnailHelper.requestBitmapAtPosition(positionMs);
//    }

//    @Override
//    public long getCurrentPosition() {
//        return mCorePlayer.getCurrentPosition();
//    }


    @Override
    public void setOnPreparedListener(OnPreparedListener listener) {
        mOutOnPreparedListener = listener;
    }

    @Override
    public void setOnRenderingStartListener(OnRenderingStartListener l) {
        mOutOnRenderingStartListener = l;
    }

    @Override
    public void setOnErrorListener(OnErrorListener listener) {
        mOutOnErrorListener = listener;
    }

    @Override
    public void setOnTrackReadyListener(OnTrackReadyListener l) {
        mOutOnTrackReadyListener = l;
    }

    @Override
    public void setOnInfoListener(OnInfoListener onInfoListener) {
        mOutOnInfoListener = onInfoListener;
    }

    @Override
    public void setOnCompletionListener(OnCompletionListener listener) {
        mOutOnCompletionListener = listener;
    }


//    private OnPCMDataListener mOutOnPcmDataListener   = null;
//    private OnPCMDataListener mInnerOnPcmDataListener = new OnPCMDataListener() {
//        @Override
//        public void onPCMData(byte[] data, int size) {
//            if (mOutOnPcmDataListener != null) {
//                mOutOnPcmDataListener.onPCMData(data, size);
//            }
//        }
//    };

//    @Override
//    public void setOnPCMDataListener(OnPCMDataListener l) {
//        mOutOnPcmDataListener = l;
//    }


    @Override
    public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener l) {
        mOutOnVideoSizeChangedListener = l;
    }

    @Override
    public void setDisplay(SurfaceHolder holder) {
        if (holder == null) {
            this.setSurface(null);
        } else {
            this.setSurface(holder.getSurface());
        }
    }

    @Override
    public void setDataSource(String url) {
        mCorePlayer.setDataSource(url);
    }

    @Override
    public void setOnLoadingStatusListener(OnLoadingStatusListener l) {
        mOutOnLoadingStatusListener = l;
    }

    @Override
    public void setOnSeekCompleteListener(OnSeekCompleteListener l) {
        mOutOnSeekEndListener = l;
    }


    @Override
    public void setOnTrackChangedListener(OnTrackChangedListener l) {
        mOutOnTrackChangedListener = l;
    }


    @Override
    public void setOnSubtitleDisplayListener(OnSubtitleDisplayListener l) {
        mOutOnSubtitleDisplayListener = l;
    }

    /**
     * 设置播放器状态变化回调监听
     *
     * @param l 播放器状态变化回调
     */
    @Override
    public void setOnStateChangedListener(OnStateChangedListener l) {
        mOutOnStatusChangedListener = l;
    }

    /**
     * 设置traceId
     *
     * @param id 用来关联各产品的ID
     */
    @Override
    public void setTraceId(String id) {
        mTraceID = id;
        mCorePlayer.setTraceId(mTraceID);
    }

    @Override
    @Deprecated
    public void redraw() {
        //empty
    }

//    @Override
//    public void setOnThumbnailListener(OnThumbnailListener l) {
//        mOutOnThumbnailListener = l;
//    }

    @Override
    public void setOnSnapShotListener(OnSnapShotListener l) {
        mOutOnSnapShotListener = l;
    }


    @Override
    public void enableHardwareDecoder(boolean enable) {
        mCorePlayer.enableHardwareDecoder(enable);
    }

    @Override
    public String getCacheFilePath(String url) {
        return mCorePlayer.getCacheFilePath(url);
    }

//    @Override
//    public String getCacheFilePath(String vid, String format, String definition, int previewTime) {
//        return mCorePlayer.getCacheFilePath(vid, format, definition, previewTime);
//    }

    @Override
    public void reload() {
        mCorePlayer.reload();
    }

    @Override
    public void addExtSubtitle(String url){
        mCorePlayer.addExtSubtitle(url);
    }

    @Override
    public void selectExtSubtitle(int index, boolean select){
        mCorePlayer.selectExtSubtitle(index,select);
    }

    @Override
    public void setDefaultBandWidth(int bandWidth) {
        mCorePlayer.setDefaultBandWidth(bandWidth);
    }

    @Override
    public Object getOption(Option key) {
        return mCorePlayer.getOption(key);
    }

    @Override
    public void setOnVideoRenderedListener(OnVideoRenderedListener listener){
        mOutOnVideoRenderedListener = listener;
        if(mOutOnVideoRenderedListener!= null) {
            mCorePlayer.setOnVideoRenderedListener(mInnerOnVideoRenderedListener);
        }else{
            mCorePlayer.setOnVideoRenderedListener(null);
        }
    }

}


