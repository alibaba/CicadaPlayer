package com.cicada.player.nativeclass;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class TrackInfo {

    /**
     * 自动切换码率的index。当选择此index，则在多码率情况下自动切换码率。
     */
    /****
     * The index of automatic bitrate switchover. When this index is specified, the media is automatically switched among multiple bitrates.
     */
    public static final int AUTO_SELECT_INDEX = -1;


    public int index;
    public Type mType;
    public String description;

    //video
    public int videoBitrate;
    public int videoWidth;
    public int videoHeight;
    public VideoHDRType videoHDRType;

    //audio
    public String audioLang;
    public int audioChannels;
    public int audioSampleRate;
    public int audioSampleFormat;

    //subtitle
    public String subtitleLang;


    //call from native
    public void setType(int mType) {
        if (mType == Type.TYPE_VIDEO.ordinal()) {
            this.mType = Type.TYPE_VIDEO;
        } else if (mType == Type.TYPE_AUDIO.ordinal()) {
            this.mType = Type.TYPE_AUDIO;
        } else if (mType == Type.TYPE_SUBTITLE.ordinal()) {
            this.mType = Type.TYPE_SUBTITLE;
        }
    }
    public void setVideoHDRType(int type) {
        if (type == VideoHDRType.VideoHDRType_SDR.ordinal()) {
            this.videoHDRType = VideoHDRType.VideoHDRType_SDR;
        } else if (type == VideoHDRType.VideoHDRType_HDR10.ordinal()) {
            this.videoHDRType = VideoHDRType.VideoHDRType_HDR10;
        }
    }

    private int nGetType(){
        return mType.ordinal();
    }

    /**
     * Track类型
     */
    /****
     * Track types
     */
    public enum Type {
        /**
         * 视频流
         */
        /****
         * Video stream.
         */
        TYPE_VIDEO,
        /**
         * 音频流
         */
        /****
         * Audio stream.
         */
        TYPE_AUDIO,
        /**
         * 字幕流
         */
        /****
         * Subtitle stream.
         */
        TYPE_SUBTITLE,
    }


    public enum VideoHDRType {
        VideoHDRType_SDR,
        VideoHDRType_HDR10
      };


    /**
     * 获取流索引
     *
     * @return 流索引
     */
    /****
     * Query the index of the stream.
     *
     * @return The index of the stream.
     */
    public int getIndex() {
        return index;
    }

    /**
     * 获取流类型
     *
     * @return 流类型
     */
    /****
     * Query the type of the stream.
     *
     * @return The type of the stream.
     */
    public Type getType() {
        return mType;
    }

    /**
     * 获取视频流码率。注意：只有在{@link #getType()} = {@link Type#TYPE_VIDEO} 时值才是正确的。
     *
     * @return 视频流码率
     */
    /****
     * Query the bitrate of the video stream. Note: Only when {@link #getType()} = {@link Type#TYPE_VIDEO}, this value is valid.
     *
     * @return The bitrate of the video stream.
     */
    public int getVideoBitrate() {
        return videoBitrate;
    }

    /**
     * 获取视频流宽度。注意：只有在{@link #getType()} = {@link Type#TYPE_VIDEO} 时值才是正确的。
     *
     * @return 视频流宽度
     */
    /****
     * Query the width of the video stream. Note: Only when {@link #getType()} = {@link Type#TYPE_VIDEO}, this value is valid.
     *
     * @return The width of the video stream.
     */
    public int getVideoWidth() {
        return videoWidth;
    }

    /**
     * 获取视频流高度。注意：只有在{@link #getType()} = {@link Type#TYPE_VIDEO} 时值才是正确的。
     *
     * @return 视频流高度
     */
    /****
     * Query the height of the video stream. Note: Only when {@link #getType()} = {@link Type#TYPE_VIDEO}, this value is valid.
     *
     * @return The height of the video stream.
     */
    public int getVideoHeight() {
        return videoHeight;
    }

    /**
     * 获取音频流语言。注意：只有在{@link #getType()} = {@link Type#TYPE_AUDIO} 时值才是正确的。
     *
     * @return 音频流语言
     */
    /****
     * Query the language of the audio stream. Note: Only when {@link #getType()} = {@link Type#TYPE_AUDIO}, this value is valid.
     *
     * @return The language of the audio stream.
     */
    public String getAudioLang() {
        return audioLang;
    }

    /**
     * 获取音频流声道数。注意：只有在{@link #getType()} = {@link Type#TYPE_AUDIO} 时值才是正确的。
     *
     * @return 音频流声道数
     */
    /****
     * Query the number of channels of the audio stream. Note: Only when {@link #getType()} = {@link Type#TYPE_AUDIO}, this value is valid.
     *
     * @return The number of channels of the audio stream.
     */
    public int getAudioChannels() {
        return audioChannels;
    }

    /**
     * 获取音频流采样率。注意：只有在{@link #getType()} = {@link Type#TYPE_AUDIO} 时值才是正确的。
     *
     * @return 音频流采样率
     */
    /****
     * Query the sampling rate of the audio stream. Note: Only when {@link #getType()} = {@link Type#TYPE_AUDIO}, this value is valid.
     *
     * @return The sampling rate of the audio stream.
     */
    public int getAudioSampleRate() {
        return audioSampleRate;
    }

    /**
     * 获取音频流采样格式。注意：只有在{@link #getType()} = {@link Type#TYPE_AUDIO} 时值才是正确的。
     *
     * @return 音频流采样格式
     */
    /****
     * Query the sampling format of the audio stream. Note: Only when {@link #getType()} = {@link Type#TYPE_AUDIO}, this value is valid.
     *
     * @return The sampling format of the audio stream.
     */
    public int getAudioSampleFormat() {
        return audioSampleFormat;
    }

    /**
     * 获取字幕流语言。注意：只有在{@link #getType()} = {@link Type#TYPE_SUBTITLE} 时值才是正确的。
     *
     * @return 字幕流语言
     */
    /****
     * Query the language of the subtitle stream. Note: Only when {@link #getType()} = {@link Type#TYPE_SUBTITLE}, this value is valid.
     *
     * @return The language of the subtitle stream.
     */
    public String getSubtitleLang() {
        return subtitleLang;
    }

    /**
     * 获取流描述。可能为空。
     *
     * @return 流描述
     */
    /****
     * Query the description of the stream. It could be empty.
     *
     * @return The stream description.
     */
    public String getDescription() {
        return description;
    }
}
