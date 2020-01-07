package com.cicada.player.demo.view.gesturedialog;

import android.app.Activity;

import com.cicada.player.demo.R;
import com.cicada.player.demo.util.VcPlayerLog;

/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 手势滑动的音量提示框。
 */
public class VolumeDialog extends BaseGestureDialog {

    private static final String TAG = VolumeDialog.class.getSimpleName();
    private int initVolume = 0;

    public VolumeDialog(Activity context, int percent) {
        super(context);
        initVolume = percent;
        mImageView.setImageResource(R.drawable.cicada_volume_img);
        updateVolume(percent);
    }

    /**
     * 更新音量值
     * @param percent 音量百分比
     */
    public void updateVolume(int percent) {
        mTextView.setText(percent + "%");
        mImageView.setImageLevel(percent);
    }

    /**
     * 获取最后的音量
     * @param changePercent 变化的百分比
     * @return 最后的音量
     */
    public int getTargetVolume(int changePercent) {

        VcPlayerLog.d(TAG, "changePercent = " + changePercent + " , initVolume  = " + initVolume);

        int newVolume = initVolume - changePercent;
        if (newVolume > 100) {
            newVolume = 100;
        } else if (newVolume < 0) {
            newVolume = 0;
        }
        return newVolume;
    }
}
