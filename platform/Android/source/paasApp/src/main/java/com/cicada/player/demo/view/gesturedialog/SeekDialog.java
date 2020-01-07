package com.cicada.player.demo.view.gesturedialog;

import android.app.Activity;

import com.cicada.player.demo.R;
import com.cicada.player.demo.util.TimeFormater;

/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 手势滑动的seek提示框。
 */
public class SeekDialog extends BaseGestureDialog {

    private int mInitPosition = 0;
    private int mFinalPosition = 0;

    public SeekDialog(Activity activity, int position) {
        super(activity);
        mInitPosition = position;
        updatePosition(mInitPosition);
    }

    public SeekDialog(Activity activity) {
        super(activity);
        updatePosition(mInitPosition);
    }

    public void updatePosition(int position) {
        //这里更新了网签和往后seek的图片
        if (position >= mInitPosition) {
            mImageView.setImageResource(R.mipmap.cicada_seek_forward);
        } else {
            mImageView.setImageResource(R.mipmap.cicada_seek_rewind);
        }
        mTextView.setText(TimeFormater.formatMs(position));
    }


    /**
     * 目标位置计算算法
     *
     * @param duration        视频总时长
     * @param currentPosition 当前播放位置
     * @param deltaPosition 与当前位置相差的时长
     * @return
     */
    public  int getTargetPosition(long duration, long currentPosition, long deltaPosition) {
        // seek步长
        long finalDeltaPosition;
        // 根据视频时长，决定seek步长
        long totalMinutes = duration / 1000 / 60;
        int hours = (int) (totalMinutes / 60);
        int minutes = (int) (totalMinutes % 60);

        // 视频时长为1小时以上，小屏和全屏的手势滑动最长为视频时长的十分之一
        if (hours >= 1) {
            finalDeltaPosition = deltaPosition / 10;
        }// 视频时长为31分钟－60分钟时，小屏和全屏的手势滑动最长为视频时长五分之一
        else if (minutes > 30) {
            finalDeltaPosition = deltaPosition / 5;
        }// 视频时长为11分钟－30分钟时，小屏和全屏的手势滑动最长为视频时长三分之一
        else if (minutes > 10) {
            finalDeltaPosition = deltaPosition / 3;
        }// 视频时长为4-10分钟时，小屏和全屏的手势滑动最长为视频时长二分之一
        else if (minutes > 3) {
            finalDeltaPosition = deltaPosition / 2;
        }// 视频时长为1秒钟至3分钟时，小屏和全屏的手势滑动最长为视频结束
        else {
            finalDeltaPosition = deltaPosition;
        }

        long targetPosition = finalDeltaPosition + currentPosition;
        if (targetPosition < 0) {
            targetPosition = 0;
        }
        if (targetPosition > duration) {
            targetPosition = duration;
        }
        mFinalPosition = (int) targetPosition;
        return mFinalPosition;
    }

    /**
     * 获取最终的位置
     * @return
     */
    public int getFinalPosition() {
        return mFinalPosition;
    }
}
