package com.cicada.player;

import android.graphics.Bitmap;
import android.view.Surface;
import android.view.SurfaceHolder;

import com.cicada.player.bean.ErrorInfo;
import com.cicada.player.bean.InfoBean;
import com.cicada.player.nativeclass.CacheConfig;
import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.PlayerConfig;
import com.cicada.player.nativeclass.TrackInfo;
/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public interface CicadaPlayer {

    /**
     * 未知状态
     */
    /****
     * Unknown status.
     */
    public static final int unknow = -1;
    /**
     * 空状态。刚创建出来的状态。
     */
    /****
     * Blank. The player enters this state after it is created.
     */
    public static final int idle = 0;
    /**
     * 初始化了的状态，设置播放源之后的状态
     */
    /****
     * Initialized. The player enters this state after a media source is specified for the player.
     */
    public static final int initalized = 1;
    /**
     * 准备成功的状态
     */
    /****
     * Prepared.
     */
    public static final int prepared = 2;
    /**
     * 正在播放的状态
     */
    /****
     * The player is playing video or audio.
     */
    public static final int started = 3;
    /**
     * 播放已暂停的状态
     */
    /****
     * The player is paused.
     */
    public static final int paused = 4;
    /**
     * 播放已停止的状态
     */
    /****
     * The player is stopped.
     */
    public static final int stopped = 5;
    /**
     * 播放完成状态
     */
    /****
     * The player has completed playing the video or audio.
     */
    public static final int completion = 6;
    /**
     * 出错状态
     */
    /****
     * The player has an error.
     */
    public static final int error = 7;


    /**
     * 选择播放的流。选择成功与否的结果通过{@link OnTrackChangedListener}回调。
     * <p>
     * 注意：自动切换码率与其他Track的选择是互斥的。选择其他Track之后，自动切换码率就失效，不起作用了。
     *
     * @param trackInfoIndex 流信息。见{@link TrackInfo#getIndex()}。
     *                       如果需要自动切换码率，则传递{@link TrackInfo#AUTO_SELECT_INDEX}.
     */
    /****
     * Specify a stream to play. You can call {@link OnTrackChangedListener} to check whether the stream is played.
     * <p>
     * Note: If you have specified a track, then automatic bitrate switchover does not take effect even if it is enabled.
     *
     * @param trackInfoIndex Stream information. See {@link TrackInfo#getIndex()}.
     *                       If you want to enable automatic bitrate switchover, pass {@link TrackInfo#AUTO_SELECT_INDEX}.
     */
    abstract public void selectTrack(int trackInfoIndex);

    /**
     * 根据type获取当前播放的流。从4.7.1开始使用{@linkplain IPlayer#currentTrack(TrackInfo.Type)}代替
     *
     * @param type 类型。见{@link TrackInfo.Type} .ordinal()
     * @return 当前播放的流。如果没有，则返回null。
     */
    /****
     * Query the playing track info according to the specified track type. From version 4.7.1, {@linkplain CicadaPlayer#currentTrack(TrackInfo.Type)} is used.
     *
     * @param type The type of the track. See {@link TrackInfo.Type} .ordinal().
     * @return The track that is playing. If no track is playing, null is returned.
     */
    @Deprecated
    abstract public TrackInfo currentTrack(int type);

    /**
     * 根据type获取当前播放的流。
     *
     * @param type 类型。见{@link TrackInfo.Type}
     * @return 当前播放的流。如果没有，则返回null。
     */
    /****
     * Query the playing track info according to the specified track type.
     *
     * @param type The type of the track. See {@link TrackInfo.Type}.
     * @return The track that is playing. If no track is playing, null is returned.
     */
    abstract public TrackInfo currentTrack(TrackInfo.Type type);

    /**
     * 获取总时长。
     *
     * @return 总时长。单位ms。
     */
    /****
     * Query the total length of the stream.
     *
     * @return The total length of the stream. Unit: milliseconds.
     */
    abstract public long getDuration();

    /**
     * 获取媒体信息。注意：只有在{@link OnPreparedListener#onPrepared()}回调之后获取才能正确。
     *
     * @return 媒体信息。见{@link MediaInfo}。
     */
    /****
     * Query media information. Note: Only if {@link OnPreparedListener#onPrepared()} is called back, the media information can be returned.
     *
     * @return The media information. See {@link MediaInfo}.
     */
    abstract public MediaInfo getMediaInfo();

    /**
     * 获取视频宽度
     *
     * @return 视频宽度
     */
    /****
     * Query the width of the video.
     *
     * @return The width of the video.
     */
    abstract public int getVideoWidth();

    /**
     * 获取视频高度
     *
     * @return 视频高度
     */
    /****
     * Query the height of the video.
     *
     * @return The height of the video.
     */
    abstract public int getVideoHeight();

    /**
     * 获取视频旋转角度
     *
     * @return 视频旋转角度
     */
    /****
     * Query the rotate angle of the video.
     *
     * @return The rotate angle of the video.
     */
    abstract public int getVideoRotation();

    /**
     * 设置缓存配置
     *
     * @param cacheConfig 缓存配置。见{@link CacheConfig}。
     */
    /****
     * Configure cache settings.
     *
     * @param cacheConfig Cache settings. See {@link CacheConfig}.
     */
    abstract public void setCacheConfig(CacheConfig cacheConfig);

    /**
     * 设置traceId 便于跟踪日志。
     *
     * @param traceId traceId
     */
    /****
     * Set a trace ID for troubleshooting with the relevant log.
     *
     * @param traceId The specified trace ID.
     */
    abstract public void setTraceId(String traceId);

    /**
     * 设置音量（非系统音量），范围0.0~2.0，当音量大于1.0时，可能出现噪音，不推荐使用。
     *
     * @param gain 范围[0,2]
     */
    /****
     * Set the volume of the player(Not system volume). The range is 0.0~2.0，it maybe lead to noise if set volume more then 1.0, not recommended.
     *
     * @param gain Valid values: [0,2].
     */
    abstract public void setVolume(float gain);

    /**
     * 获取音量。
     *
     * @return 范围[0, 1]
     */
    /****
     * Query the volume of the player.
     *
     * @return Valid values: [0, 1].
     */
    abstract public float getVolume();

    /**
     * 设置静音
     *
     * @param on true:静音。默认false。
     */
    /****
     * Mute the player.
     *
     * @param on Set to true to mute the player. Default: false.
     */
    abstract public void setMute(boolean on);

    /**
     * 是否静音
     *
     * @return true:静音。默认false。
     */
    /**
     * Query whether the player is muted.
     *
     * @return Value true indicates that the player is muted. Default: false.
     */
    abstract public boolean isMute();

    /**
     * 设置循环播放。如果本地有了缓存，那么下次循环播放则播放本地文件。
     *
     * @param on true：开启循环播放。默认关闭。
     */
    /****
     * Enable loop playback. If the media file is already downloaded to the local host, then the downloaded media file will be used for loop playback.
     *
     * @param on true：Enable loop playback. Default: disabled.
     */
    abstract public void setLoop(boolean on);

    /**
     * 是否循环播放
     *
     * @return true：开启了循环播放。默认关闭。
     */
    /****
     * Query whether loop playback is enabled.
     *
     * @return Value true indicates that loop playback is enabled. Default: disabled.
     */
    abstract public boolean isLoop();

    /**
     * 设置自动播放。如果是自动播放，则接受不到{@link OnPreparedListener}回调。
     *
     * @param on true：是。默认否。
     */
    /****
     * Enable autoplay. If autoplay is enabled, then the {@link OnPreparedListener} callback is not returned.
     *
     * @param on Value true indicates that autoplay is enabled. Default: disabled.
     */
    abstract public void setAutoPlay(boolean on);

    /**
     * 是否自动播放。
     *
     * @return true：是。默认否。
     */
    /****
     * Query whether autoplay is enabled.
     *
     * @return Value true indicates that autoplay is enabled. Default: disabled.
     */
    abstract public boolean isAutoPlay();

    /**
     * 设置倍数播放。
     *
     * @param speed 范围[0.5,2]
     */
    /****
     * Set the playback speed.
     *
     * @param speed Valid values: [0.5,2].
     */
    abstract public void setSpeed(float speed);

    /**
     * 获取倍数播放值。
     *
     * @return 倍数播放值。范围[0.5,2]
     */
    /****
     * Query the playback speed.
     *
     * @return The playback speed. Valid values: [0.5,2].
     */
    abstract public float getSpeed();
    /**
     * 镜像模式
     */
    /****
     * Mirroring modes
     */
    public static enum MirrorMode {
        /**
         * 无镜像
         */
        /****
         * Disable mirroring.
         */
        MIRROR_MODE_NONE(0),
        /**
         * 水平镜像
         */
        /****
         * Horizontal mirroring.
         */
        MIRROR_MODE_HORIZONTAL(1),
        /**
         * 垂直镜像
         */
        /****
         * Vertical mirroring.
         */
        MIRROR_MODE_VERTICAL(2);

        private int mValue;

        private MirrorMode(int value) {
            mValue = value;
        }

        /**
         * 获取对应的值
         *
         * @return 值。
         */
        /****
         * Query the value of the specified parameter.
         *
         * @return The returned value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /**
     * 设置镜像模式
     *
     * @param mirrorMode 镜像模式。 见{@link MirrorMode}。
     */
    /****
     * Set a mirroring mode
     *
     * @param mirrorMode The specified mirroring mode. See {@link MirrorMode}.
     */
    abstract public void setMirrorMode(MirrorMode mirrorMode);

    /**
     * 获取当前镜像模式。
     *
     * @return 镜像模式。 见{@link MirrorMode}。
     */
    /****
     * Query the current mirroring mode.
     *
     * @return The current mirroring mode. See {@link MirrorMode}.
     */
    abstract public MirrorMode getMirrorMode();

    /**
     * 旋转模式
     */
    /****
     * Rotate modes
     */
    public static enum RotateMode {
        /**
         * 顺时针旋转0度
         */
        /****
         * Do not rotate.
         */
        ROTATE_0(0),
        /**
         * 顺时针旋转90度
         */
        /****
         * Rotate 90 degree clockwise.
         */
        ROTATE_90(90),
        /**
         * 顺时针旋转180度
         */
        /****
         * Rotate 180 degree clockwise.
         */
        ROTATE_180(180),
        /**
         * 顺时针旋转270度
         */
        /****
         * Rotate 270 degree clockwise.
         */
        ROTATE_270(270);

        private int mValue;

        private RotateMode(int value) {
            mValue = value;
        }

        /**
         * 获取对应的值
         *
         * @return 值。
         */
        /****
         * Query the value of the specified parameter.
         *
         * @return The returned value.
         */
        public int getValue() {
            return mValue;
        }

    }

    /**
     * 设置画面旋转模式
     *
     * @param rotateMode 旋转模式。见{@link RotateMode}
     */
    /****
     * Set a rotate mode.
     *
     * @param rotateMode The specified rotate mode. See {@link RotateMode}.
     */
    abstract public void setRotateMode(RotateMode rotateMode);

    /**
     * 获取画面旋转模式
     *
     * @return 旋转模式。见{@link RotateMode}
     */
    /****
     * Query the current rotate mode.
     *
     * @return The current rotate mode. See {@link RotateMode}.
     */
    abstract public RotateMode getRotateMode();

    /**
     * 缩放模式
     */
    /****
     * Zoom modes
     */
    public static enum ScaleMode {

        /**
         * 宽高比适应
         */
        /****
         * Auto zoom to fit.
         */
        SCALE_ASPECT_FIT(0),
        /**
         * 宽高比填充
         */
        /****
         * Fill to fit.
         */
        SCALE_ASPECT_FILL(1),
        /**
         * 拉伸填充
         */
        /****
         * Stretch to fit.
         */
        SCALE_TO_FILL(2);


        private int mValue;

        private ScaleMode(int value) {
            mValue = value;
        }

        /**
         * 获取对应的值
         *
         * @return 值。
         */
        /****
         * Query the value of the specified parameter.
         *
         * @return The returned value.
         */
        public int getValue() {
            return mValue;
        }
    }

    /**
     * 设置画面缩放模式
     *
     * @param scaleMode 缩放模式。默认{@link ScaleMode#SCALE_TO_FILL}. 见{@link ScaleMode}。
     */
    /****
     * Set a zoom mode.
     *
     * @param scaleMode The specified zoom mode. Default: {@link ScaleMode#SCALE_TO_FILL}. See {@link ScaleMode}.
     */
    abstract public void setScaleMode(ScaleMode scaleMode);

    /**
     * 获取画面缩放模式
     *
     * @return 缩放模式。默认{@link ScaleMode#SCALE_TO_FILL}.
     */
    /****
     * Query the current zoom mode.
     *
     * @return The current zoom mode. Default: {@link ScaleMode#SCALE_TO_FILL}.
     */
    abstract public ScaleMode getScaleMode();

    /**
     * 截取当前画面.截图结果通过{@link OnSnapShotListener}回调。
     */
    /****
     * Create a snapshot for the current frame. You can call {@link OnSnapShotListener} to return the snapshot creation result.
     */
    abstract public void snapshot();

    /**
     * 设置播放的surface。注意:画面变化时也需要设置。销毁时需设置为null。
     *
     * @param surface surface。
     */
    /****
     * Set the display surface. Note: You must set this parameter when the display surface changes. You must set this parameter to null when the player is deleted.
     *
     * @param surface The display surface.
     */
    abstract public void setSurface(Surface surface);

    /**
     * 设置播放的SurfaceHolder。注意:画面变化时也需要设置。销毁时需设置为null。
     *
     * @param holder SurfaceHolder。
     */
    /****
     * Set SurfaceHolder. Note: You must set this parameter when the display surface changes. You must set this parameter to null when the player is deleted.
     *
     * @param holder The SurfaceHolder.
     */
    abstract public void setDisplay(SurfaceHolder holder);

    /**
     * 设置Url数据源
     *
     * @param url 本机地址或网络地址。
     */
	 /****
      * set up the Url data source
      * @param url local or network address.
      */
    void setDataSource(String url);

    /**
     * 准备。成功结果通过{@link OnPreparedListener}回调，或者失败{@link OnErrorListener}
     */
    /****
     * Prepare the player. Call {@link OnPreparedListener} to return success messages. Call {@link OnErrorListener} to return error messages.
     */
    abstract public void prepare();

    /**
     * 开始播放。
     */
    /****
     * Start the player.
     */
    abstract public void start();

    /**
     * 暂停播放
     */
    /****
     * Pause the player.
     */
    abstract public void pause();

    /**
     * 停止播放
     */
    /****
     * Stop the player.
     */
    abstract public void stop();

    /**
     * seek模式
     */
    /****
     * Seeking modes
     */
    public static enum SeekMode {
        /**
         * 精准seek
         */
        /****
         * Accurate seeking.
         */
        Accurate(0x01),
        /**
         * 不精准seek
         */
        /****
         * Inaccurate seeking.
         */
        Inaccurate(0x10),
        ;

        private int mValue;

        private SeekMode(int value) {
            mValue = value;
        }

        public int getValue() {
            return mValue;
        }
    }

    /**
     * 跳转到。不精准。
     *
     * @param position 位置。单位毫秒。
     */
    /****
     * Specify a position for inaccurate seeking. Unit: millisecond.
     *
     * @param position The specified position.
     */
    abstract public void seekTo(long position);

    /**
     * 跳转到。
     *
     * @param position 位置。单位毫秒。
     * @param seekMode seek模式。见{@link SeekMode}。
     */
    /****
     * Seek to a specified position.
     *
     * @param position The specified position. Unit: millisecond.
     * @param seekMode The specified seeking mode. See {@link SeekMode}.
     */
    abstract public void seekTo(long position, SeekMode seekMode);

    /**
     * 设置精准seek的最大间隔。
     * @param delta 间隔时间，单位毫秒
     */

    /****
    * set the maximum interval of precision seek.
    * @param delta interval in milliseconds
    */

    abstract public void setMaxAccurateSeekDelta(int delta);

    /**
     * 重置。
     */
    /****
     * Reset.
     */
    abstract public void reset();

    /**
     * 释放。
     */
    /****
     * Release.
     */
    abstract public void release();

    /**
     * 刷新一帧画面。在surface的大小变化的时候，可保留最后一帧。
     */
	/****
     * Refresh a frame. The last frame is retained when the size of the display surface changes.
     */
    @Deprecated
    abstract public void redraw();

    /**
     * 准备成功通知
     */
    /****
     * Preparation success callback.
     */
    public interface OnPreparedListener {
        /**
         * 准备成功
         */
        /****
         * Preparation is complete.
         */
        void onPrepared();
    }

    /**
     * 设置准备成功通知。如果失败，则会通知{@link OnErrorListener}。
     *
     * @param l 准备成功通知
     */
    /****
     * Set a preparation success callback. If the preparation failed, the {@link OnErrorListener} is triggered.
     *
     * @param l Preparation success notification.
     */
    abstract public void setOnPreparedListener(OnPreparedListener l);

    /**
     * 渲染开始通知
     */
    /****
     * Rendering start callback.
     */
    public interface OnRenderingStartListener {
        /**
         * 渲染开始。
         */
        /****
         * Rendering starts.
         */
        void onRenderingStart();
    }

    /**
     * 设置渲染开始通知。可以监听首帧显示事件等，用于隐藏封面等功能。
     *
     * @param l 渲染开始通知。
     */
    /****
     * Set a rendering start callback. You can use this callback to listen to first frame display events and hide the album cover.
     *
     * @param l Rendering start notification.
     */
    abstract public void setOnRenderingStartListener(OnRenderingStartListener l);

    /**
     * 播放器状态变化通知
     */
    /****
     * Player status update callback.
     */
    public interface OnStateChangedListener {
        /**
         * 状态变化
         *
         * @param newState 新状态
         */
        /****
         * The player status is updated.
         *
         * @param newState The updated status.
         */
        void onStateChanged(int newState);
    }

    /**
     * 设置播放器状态变化通知
     *
     * @param l 播放器状态变化通知
     */
    /****
     * Set a player status update callback.
     *
     * @param l Player status update notification.
     */
    abstract public void setOnStateChangedListener(OnStateChangedListener l);

    /**
     * 播放完成通知.
     */
    /****
     * Playback completion callback.
     */
    public interface OnCompletionListener {
        /**
         * 播放完成
         */
        /****
         * The player has completed playing the video or audio.
         */
        void onCompletion();
    }

    /**
     * 设置播放完成通知.注意：循环播放不会发出此通知。
     *
     * @param l 播放完成通知.
     */
    /****
     * Set a playback completion callback. Note: No notification is sent if loop playback is enabled.
     *
     * @param l Playback completion notification.
     */
    abstract public void setOnCompletionListener(OnCompletionListener l);

    /**
     * 加载状态通知
     */
    /****
     * Loading status callback.
     */
    public interface OnLoadingStatusListener {
        /**
         * 开始加载。
         */
        /****
         * Start loading.
         */
        void onLoadingBegin();

        /**
         * 加载进度
         *
         * @param percent  百分比，[0,100]
         * @param netSpeed 当前网速。kbps
         */
        /****
         * Loading progress.
         *
         * @param percent  The loading progress in percentage. Valid values: [0,100].
         * @param netSpeed The current bandwidth. Unit: kbit/s.
         */
        void onLoadingProgress(int percent, float netSpeed);  

        /**
         * 加载结束
         */
        /****
         * Loading is complete.
         */
        void onLoadingEnd();
    }

    /**
     * 设置加载状态通知。
     *
     * @param l 加载状态通知
     */
    /****
     * Set a loading status callback.
     *
     * @param l Loading status notification.
     */
    abstract public void setOnLoadingStatusListener(OnLoadingStatusListener l);

    /**
     * 出错通知
     */
    /****
     * Error callback.
     */
    public interface OnErrorListener {
        /**
         * 出错
         *
         * @param errorInfo 错误信息
         */
        /****
         * An error occurs.
         *
         * @param errorInfo Error message.
         */
        void onError(ErrorInfo errorInfo);
    }

    /**
     * 设置出错通知
     *
     * @param l 出错通知
     */
    /****
     * Set an error callback.
     *
     * @param l Error message.
     */
    abstract public void setOnErrorListener(OnErrorListener l);

    /**
     * 流准备完成通知
     */
    /****
     * Stream preparation success callback.
     */
    public interface OnTrackReadyListener {
        /**
         * 流准备完成
         *
         * @param mediaInfo 媒体信息。见{@link MediaInfo}
         */
        /****
         * The stream is ready.
         *
         * @param mediaInfo Media information. See {@link MediaInfo}.
         */
        void onTrackReady(MediaInfo mediaInfo);
    }

    /**
     * 设置流准备完成通知
     *
     * @param l 流准备完成通知
     */
    /****
     * Set a stream preparation success callback.
     *
     * @param l Stream preparation success notification.
     */
    abstract public void setOnTrackReadyListener(OnTrackReadyListener l);

    /**
     * 信息通知
     */
    /****
     * Notification callback.
     */
    public interface OnInfoListener {
        /**
         * 信息
         *
         * @param infoBean 信息对象。见{@linkplain InfoBean}
         */
        /****
         * Indicate a notification.
         *
         * @param infoBean The notification object. See {@linkplain InfoBean}.
         */
        void onInfo(InfoBean infoBean);
    }

    /**
     * 设置信息监听
     *
     * @param l 信息监听
     */
    /****
     * Set a notification callback.
     *
     * @param l The notification.
     */
    abstract public void setOnInfoListener(OnInfoListener l);

    /**
     * 视频宽高变化通知
     */
    /****
     * Video size change callback.
     */
    public interface OnVideoSizeChangedListener {
        /**
         * 视频宽高变化
         *
         * @param width  宽
         * @param height 高
         */
        /****
         * Video size changes.
         *
         * @param width  Width.
         * @param height Height.
         */
        void onVideoSizeChanged(int width, int height);
    }

    /**
     * 设置视频宽高变化通知
     *
     * @param l 视频宽高变化通知
     */
    /****
     * Set a video size change callback.
     *
     * @param l Video size change notification.
     */
    abstract public void setOnVideoSizeChangedListener(OnVideoSizeChangedListener l);

    /**
     * 拖动完成通知
     */
    /****
     * Seeking completion callback.
     */
    public interface OnSeekCompleteListener {
        /**
         * 拖动完成
         */
        /****
         * Seeking is complete.
         */
        void onSeekComplete();
    }

    /**
     * 设置拖动完成通知
     *
     * @param l 拖动完成通知
     */
    /****
     * Set a seeking completion callback.
     *
     * @param l Seeking completion notification.
     */
    abstract public void setOnSeekCompleteListener(OnSeekCompleteListener l);

    /**
     * 切换流变化通知。
     */
    /****
     * Stream switchover callback.
     */
    public interface OnTrackChangedListener {
        /**
         * 切换成功
         *
         * @param trackInfo 流信息。见{@link TrackInfo}
         */
        /****
         * The stream is switched.
         *
         * @param trackInfo Stream information. See {@link TrackInfo}.
         */
        void onChangedSuccess(TrackInfo trackInfo);

        /**
         * 切换失败
         *
         * @param trackInfo 流信息。见{@link TrackInfo}
         * @param errorInfo 错误信息。见{@link ErrorInfo}
         */
        /****
         * Failed to switch the stream.
         *
         * @param trackInfo Stream information. See {@link TrackInfo}.
         * @param errorInfo Error message. See {@link ErrorInfo}.
         */
        void onChangedFail(TrackInfo trackInfo, ErrorInfo errorInfo);
    }

    /**
     * 设置流变化通知
     *
     * @param l 流变化通知
     */
    /****
     * Set a stream switchover callback.
     *
     * @param l Stream switchover notification.
     */
    abstract public void setOnTrackChangedListener(OnTrackChangedListener l);

    /**
     * 字幕显示通知
     */
    /****
     * Subtitle display callback.
     */
    public interface OnSubtitleDisplayListener {

        /**
         * 外挂字幕添加成功
         * @param trackIndex 流id
         * @param url 地址
         */
		/****
		 * external subtitles added successfully
         * @param trackIndex stream id
         * @param url
         */
        void onSubtitleExtAdded(int trackIndex,String url);
		
        /**
         * 显示字幕
         *
         * @param trackIndex 流id
         * @param id   索引
         * @param data 内容
         */
        /****
         * Show subtitles.
         * 
		 * @param trackIndex Track index.
         * @param id   Subtitle index.
         * @param data Content.
         */
        void onSubtitleShow(int trackIndex ,long id, String data);

        /**
         * 隐藏字幕
         *
         * @param trackIndex 流id
         * @param id 索引
         */
		/****
         * Hide subtitles.
		 *
         * @param trackIndex Track index.
         * @param id Subtitle index.
         */
        void onSubtitleHide(int trackIndex, long id);

    }

    /**
     * 添加外挂字幕
     * @param url 字幕地址
     */
    /****
     * Add external subtitles
     * @param url subtitle address
     */
    abstract public void addExtSubtitle(String url);

    /**
     * 选择外挂字幕
     * @param trackIndex 字幕索引
     * @param select true：选择，false：关闭
     */
    /****
     * Select external subtitles
     * @param trackIndex caption index
     * @param select true: select, false: close
     */
    abstract public void selectExtSubtitle(int trackIndex, boolean select);

    /**
     * 设置字幕显示通知
     *
     * @param l 字幕显示通知
     */
    /****
     * Set a subtitle display callback.
     *
     * @param l Subtitle display notification.
     */
    abstract public void setOnSubtitleDisplayListener(OnSubtitleDisplayListener l);

    /**
     * 设置播放器配置。
     *
     * @param config 播放器配置。见{@link PlayerConfig}
     */
    /****
     * Modify the player configuration.
     *
     * @param config The configuration of the player. See {@link PlayerConfig}.
     */
    abstract public void setConfig(PlayerConfig config);

    /**
     * 获取播放器配置
     *
     * @return 播放器配置
     */
    /****
     * Query the player configuration.
     *
     * @return The player configuration.
     */
    abstract public PlayerConfig getConfig();

    /**
     * 截图结果。
     */
    /****
     * Snapshot creation result callback.
     */
    interface OnSnapShotListener {
        /**
         * 截图成功的回调
         *
         * @param bm     图片。 注意:使用完之后注意释放recycle。
         * @param with   宽度
         * @param height 高度
         */
        /****
         * Snapshot creation success callback.
         *
         * @param bm     The snapshot. Note: After you use the snapshot, call the recycle method to release it.
         * @param with   Width.
         * @param height Height.
         */
        void onSnapShot(Bitmap bm, int with, int height);
    }

    /**
     * 截图结果监听
     *
     * @param l 截图结果事件
     */
    /****
     * Set a snapshot creation result callback.
     *
     * @param l Snapshot creation notification.
     */
    abstract public void setOnSnapShotListener(OnSnapShotListener l);

    /**
     * 是否启用硬解码
     *
     * @param enable true:启用。false:关闭。
     */
    /****
     * Enable or disable hardware decoding.
     *
     * @param enable Set to true to enable hardware decoding and set to false to disable hardware decoding.
     */
    abstract public void enableHardwareDecoder(boolean enable);

    /**
     * 转换url回调接口。注意：在回调中需要同步操作。
     */
    /****
     * URL conversion callback. Note: You must set the callback to sync.
     */
    public static interface ConvertURLCallback {
        /**
         * @param srcURL    原始URL
         * @param srcFormat 原始格式
         * @return 转换后的URL。如果失败，返回NULL
         */
        /****
         * @param srcURL    The original URL.
         * @param srcFormat The original format.
         * @return The converted URL. If URL conversion failed, NULL is returned.
         */
        String convertURL(String srcURL, String srcFormat);
    }

    /**
     * 根据url获取缓存的文件名。必须先调用setCacheConfig才能获取到。
     *
     * @param URL URL
     * @return 最终缓存的文件绝对路径。
     */
    /****
     * Query the name of a cached file with the specified URL. You must first call setCacheConfig.
     *
     * @param URL The URL of the cached file.
     * @return The absolute path of the cached file.
     */
    abstract public String getCacheFilePath(String URL);

    /**
     * 重新加载。比如网络超时时，可以重新加载。
     */
    /****
     * Reload. Call this method when the network connection times out.
     */
    abstract public void reload();

    /**
     * 设置多码率时默认播放的码率。将会选择与之最接近的一路流播放。
     * @param bandWidth 播放的码率。
     */
    /****
     * Set the default playback bitrate for multi-bit rate. The nearest stream will be selected.
     * @param bandWidth bit rate .
     */
    abstract public void setDefaultBandWidth(int bandWidth);

    /**
     *
     */
    public static class Option {
        /**
         * 渲染的fps。类型为Float
         */
        /****
         * render fps. Return value type is Float
         */
        public static Option RenderFPS = new Option("renderFps");

        private String mValue;

        private Option(String value) {
            mValue = value;
        }

        public String getValue() {
            return mValue;
        }
    }

    /**
     * 获取播放器的参数
     *
     * @param key 参数值
     * @return
     */
    abstract public Object getOption(Option key);

    public interface OnVideoRenderedListener{
        void onVideoRendered(long timeMs , long pts);
    }

    abstract public void setOnVideoRenderedListener(OnVideoRenderedListener l);
}
