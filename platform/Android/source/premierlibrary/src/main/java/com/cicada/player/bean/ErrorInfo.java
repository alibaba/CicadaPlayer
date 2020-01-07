package com.cicada.player.bean;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class ErrorInfo {
    private ErrorCode mCode;
    private String    mMsg;
    private String    mExtra;

    /**
     * 获取错误码
     *
     * @return 错误码
     */
    /****
     * Query error codes.
     *
     * @return The returned error codes.
     */
    public ErrorCode getCode() {
        return mCode;
    }

    /**
     * 设置错误码
     *
     * @param mCode 错误码
     */
    /****
     * Set error codes.
     *
     * @param mCode Error codes.
     */
    public void setCode(ErrorCode mCode) {
        this.mCode = mCode;
    }

    /**
     * 获取错误消息
     *
     * @return 错误消息
     */
    /****
     * Query error messages.
     *
     * @return Error messages.
     */
    public String getMsg() {
        return mMsg;
    }

    /**
     * 设置错误消息
     *
     * @param mMsg 错误消息
     */
    /****
     * Set error messages.
     *
     * @param mMsg Error messages.
     */
    public void setMsg(String mMsg) {
        this.mMsg = mMsg;
    }

    /**
     * 获取额外信息
     *
     * @return 额外信息
     */
    /****
     * Query additional information.
     *
     * @return Additional information.
     */
    public String getExtra() {
        return mExtra;
    }

    /**
     * 设置额外信息
     *
     * @param mExtra 额外信息
     */
    /****
     * Set additional information.
     *
     * @param mExtra Additional information.
     */
    public void setExtra(String mExtra) {
        this.mExtra = mExtra;
    }
}
