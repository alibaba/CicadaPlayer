package com.cicada.player.bean;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public enum InfoCode {
    /**
     * 未知。
     */
    /****
     * Unknown error.
     */
    Unknown(-1),

    /**
     * 循环播放开始。无额外信息。
     */
    /****
     * Loop playback is started. No additional information is provided.
     */
    LoopingStart(0),

    /**
     * 缓冲位置。额外值为当前缓冲位置。单位：毫秒。
     */
    /****
     * Buffer position. Additional value: the current buffer position. Unit: milliseconds.
     */
    BufferedPosition(1),
    /**
     * 当前播放位置。额外值为当前播放位置。单位：毫秒。
     */
    /****
     * Playback position. Additional value: the current playback position. Unit: milliseconds.
     */
    CurrentPosition(2),
    /**
     * 开始自动播放。无额外信息。
     */
    /****
     * Autoplay is started. No additional information is provided.
     */
    AutoPlayStart(3),

    /**
     * 设置了硬解，但是切换为软解。额外信息为描述信息。
     */
    /****
     * Hardware decoding is switched to software decoding. Additional information: description.
     */
    SwitchToSoftwareVideoDecoder(100),
    /**
     * 音频解码格式不支持。额外信息为描述信息。
     */
    /****
     * Audio decoding does not support the specified format. Additional information: description.
     */
    AudioCodecNotSupport(101),
    /**
     * 音频解码器设备失败。额外信息为描述信息。
     */
    /****
     * The audio decoder failed. Additional information: description.
     */
    AudioDecoderDeviceError(102),
    /**
     * 视频解码格式不支持。额外信息为描述信息。
     */
    /****
     * Video decoding does not support the specified format. Additional information: description.
     */
    VideoCodecNotSupport(103),
    /**
     * 视频解码器设备失败。额外信息为描述信息。
     */
    /****
     * The video decoder failed. Additional information: description.
     */
    VideoDecoderDeviceError(104),
    /**
     * 视频渲染设备初始化失败。额外信息为描述信息。
     */
    /****
     * The video renderer failed to initialize. Additional information: description.
     */
    VideoRenderInitError(105),

    /**
     * Demuxer trace ID for artp。
     */
    /****
     * Demuxer trace ID for artp.
     */
    DemuxerTraceID(106),

    /**
     * 网络失败，需要重试。无额外信息。
     */
    /****
     * Network connection failed. Try again. No additional information is provided.
     */
    NetworkRetry(108),

    /**
     * 缓存成功。无额外信息。
     */
    /****
     * Content cached. No additional information is provided.
     */
    CacheSuccess(109),

    /**
     * 缓存失败。额外信息为描述信息。
     */
    /****
     * Failed to cache the content. Additional information: description.
     */
    CacheError(110),

    /**
     * 系统无可用内存来存放媒体数据
     */
    /****
     * The system does not have memory to cache the media data.
     */
    LowMemory(111),

    /**
     * 网络重试成功。无额外信息。
     */
	/****
	 * Network retry successful. No additional information.
	 */
    NetworkRetrySuccess(113),

    /**
     * 字幕选择失败。额外信息为失败原因。
     */
	/****
	 * Subtitle selection failed. Additional information is the cause of the failure.
	 */
    SubtitleSelectError(114),

    /**
     * 播放组件中透传输出的消息
     */
    /****
     * The message passthrough the player component
     */

    DirectComponentMSG(116)

            ;

    private int value;

    private InfoCode(int value) {
        this.value = value;
    }

    public int getValue() {
        return value;
    }


}
