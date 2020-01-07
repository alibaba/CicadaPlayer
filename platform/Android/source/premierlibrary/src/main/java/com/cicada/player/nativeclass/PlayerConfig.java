package com.cicada.player.nativeclass;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class PlayerConfig {

    private PlayerConfig() {

    }

    /**
     * http代理。例如："http://ip:port"
     */
    /****
     * HTTP proxy. Example: "http://ip:port".
     */
    public String mHttpProxy = "";

    /**
     * referrer
     */
    /****
     * Referer.
     */
    public String mReferrer = "";

    /**
     * 网络超时时间。单位ms。
     */
    /****
     * Network connection timeout time. Unit: milliseconds.
     */
    public int mNetworkTimeout = 15000;

    /**
     * 最大延迟。注意：直播有效。
     */
    /****
     * Maximum delay time. Note: This parameter is only valid for broadcasting.
     */
    public int mMaxDelayTime = 5000;

    /**
     * 最大缓冲区时长。单位ms
     */
    /****
     * Maximum buffer size. Unit: milliseconds.
     */
    public int mMaxBufferDuration = 50000;

    /**
     * 高缓冲时长。单位ms
     */
    /****
     * Maximum buffer time. Unit: milliseconds.
     */
    public int mHighBufferDuration = 3000;

    /**
     * 起播缓冲区时长。单位ms。
     */
    /****
     * The size of the data that the player must buffer before it can start playing the media. Unit: milliseconds.
     */
    public int mStartBufferDuration = 500;
    /**
     * 最大probe大小。单位 byte 。 -1：默认。
     */
    /****
     * Maximum probe size. Unit: bytes. Default: -1.
     */
    public int mMaxProbeSize = -1;

    /**
     * 停止后是否清空画面。默认为false。
     */
    /****
     * Indicate whether to clear the player surface when playback is stopped. Default: false.
     */
    public boolean mClearFrameWhenStop = false;

    /**
     * 是否启用TunnelRender
     */
    /****
     * Indicate whether TunnelRender is enabled.
     */
    public boolean mEnableVideoTunnelRender = false;

    /**
     * 设置请求的ua
     */
    /****
     * Set a request UserAgent.
     */
    public String mUserAgent = "";

    /**
     * 网络重试次数，每次间隔networkTimeout，networkRetryCount=0则表示不重试，重试策略app决定，默认值为2
     */
    /****
     * The maximum network reconnection attempts. networkTimeout specifies the reconnection interval. networkRetryCount=0 indicates that automatic reconnection is disabled. The reconnection policy depends on the app. Default: 2.
     */
    public int mNetworkRetryCount = 2;

    private String[] mCustomHeaders = null;

    public String[] getCustomHeaders() {
        return mCustomHeaders;
    }

    public void setCustomHeaders(String[] headers) {
        mCustomHeaders = headers;
    }
}
