package com.cicada.player.demo.view.quality;

import android.content.Context;


/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 清晰度列表的项
 */
public class QualityItem {
    //原始的清晰度
    private String mQuality;
    //显示的文字
    private String mName;

    private QualityItem(String quality, String name) {
        mQuality = quality;
        mName = name;
    }

    public static QualityItem getItem(Context context, String quality, boolean isMts) {
//mts与其他的清晰度格式不一样，
        if (isMts) {
            //这里是getMtsLanguage
            return new QualityItem(quality, quality);
        } else {
            //这里是getSaasLanguage
            return new QualityItem(quality, quality);
        }
    }

    /**
     * 获取显示的文字
     *
     * @return 清晰度文字
     */
    public String getName() {
        return mName;
    }

}
