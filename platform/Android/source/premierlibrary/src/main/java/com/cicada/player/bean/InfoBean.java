package com.cicada.player.bean;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class InfoBean {

    private InfoCode mCode;
    private long     mExtraValue = -1;
    private String   mExtraMsg   = null;

    /**
     * 获取info的类型
     * @return 类型
     */
    /****
     * Query the type of info.
     * @return The type of info.
     */
    public InfoCode getCode() {
        return mCode;
    }

    /**
     * 设置类型
     * @param mCode 设置类型
     */
    /****
     * Set the info type.
     * @param mCode The info type.
     */
    public void setCode(InfoCode mCode) {
        this.mCode = mCode;
    }

    /**
     * 获取额外值.
     * @return 额外值。具体参考{@linkplain InfoCode}的说明。
     */
    /****
     * Query additional values.
     * @return Additional values. See {@linkplain InfoCode}.
     */
    public long getExtraValue() {
        return mExtraValue;
    }

    /**
     * 设置额外值
     * @param mExtraValue 额外值
     */
    /****
     * Set additional values.
     * @param mExtraValue Additional values.
     */
    public void setExtraValue(long mExtraValue) {
        this.mExtraValue = mExtraValue;
    }

    /**
     * 获取额外信息
     * @return 额外信息。具体参考{@linkplain InfoCode}的说明。
     */
    /****
     * Query additional information.
     * @return Additional information. See {@linkplain InfoCode}.
     */
    public String getExtraMsg() {
        return mExtraMsg;
    }

    /**
     * 设置额外信息
     * @param mExtraMsg 额外信息
     */
    /****
     * Set additional information.
     * @param mExtraMsg Additional information.
     */
    public void setExtraMsg(String mExtraMsg) {
        this.mExtraMsg = mExtraMsg;
    }
}
