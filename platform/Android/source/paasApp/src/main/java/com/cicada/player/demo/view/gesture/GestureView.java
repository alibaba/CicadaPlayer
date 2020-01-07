package com.cicada.player.demo.view.gesture;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;

import com.cicada.player.demo.listener.ViewAction;
import com.cicada.player.demo.bean.CicadaScreenMode;
import com.cicada.player.demo.util.VcPlayerLog;
/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 手势滑动的view。用于UI中处理手势的滑动事件，从而去实现手势改变亮度，音量，seek等操作。
 */
public class GestureView extends View implements ViewAction {

    private static final String TAG = GestureView.class.getSimpleName();

    //手势控制
    protected GestureControl mGestureControl;
    //监听器
    private GestureListener mOutGestureListener = null;

    //隐藏原因
    private HideType mHideType = null;
    //是否锁定屏幕
    private boolean mIsOnlySingleTapEnable = false;

    public GestureView(Context context) {
        super(context);
        init();
    }

    public GestureView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public GestureView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        //创建手势控制
        mGestureControl = new GestureControl(this);
        //设置监听
        mGestureControl.setOnGestureControlListener(new GestureListener() {

            @Override
            public void onHorizontalDistance(float downX, float nowX) {
                //其他手势如果锁住了就不回调了。
                if (mIsOnlySingleTapEnable) {
                    return;
                }
                if (mOutGestureListener != null) {
                    mOutGestureListener.onHorizontalDistance(downX, nowX);
                }
            }

            @Override
            public void onLeftVerticalDistance(float downY, float nowY) {
                //其他手势如果锁住了就不回调了。
                if (mIsOnlySingleTapEnable) {
                    return;
                }
                if (mOutGestureListener != null) {
                    mOutGestureListener.onLeftVerticalDistance(downY, nowY);
                }
            }

            @Override
            public void onRightVerticalDistance(float downY, float nowY) {
                //其他手势如果锁住了就不回调了。
                if (mIsOnlySingleTapEnable) {
                    return;
                }
                if (mOutGestureListener != null) {
                    mOutGestureListener.onRightVerticalDistance(downY, nowY);
                }
            }

            @Override
            public void onGestureEnd() {
                //其他手势如果锁住了就不回调了。
                if (mIsOnlySingleTapEnable) {
                    return;
                }
                if (mOutGestureListener != null) {
                    mOutGestureListener.onGestureEnd();
                }
            }

            @Override
            public void onSingleTap() {
                //锁屏的时候，单击还是有用的。。不然没法显示锁的按钮了
                if (mOutGestureListener != null) {
                    mOutGestureListener.onSingleTap();
                }
            }

            @Override
            public void onDoubleTap() {
                //其他手势如果锁住了就不回调了。
                if (mIsOnlySingleTapEnable) {
                    return;
                }

                if (mOutGestureListener != null) {
                    mOutGestureListener.onDoubleTap();
                }
            }

        });
    }

    /**
     * 设置是否锁定全屏了。锁定全屏的话，除了单击手势有响应，其他都不会有响应。
     *
     * @param locked true：锁定。
     */
    public void setScreenLockStatus(boolean locked) {
        mIsOnlySingleTapEnable = locked;
    }


    public interface GestureListener {
        /**
         * 水平滑动距离
         *
         * @param downX 按下位置
         * @param nowX  当前位置
         */
        void onHorizontalDistance(float downX, float nowX);

        /**
         * 左边垂直滑动距离
         *
         * @param downY 按下位置
         * @param nowY  当前位置
         */
        void onLeftVerticalDistance(float downY, float nowY);

        /**
         * 右边垂直滑动距离
         *
         * @param downY 按下位置
         * @param nowY  当前位置
         */
        void onRightVerticalDistance(float downY, float nowY);

        /**
         * 手势结束
         */
        void onGestureEnd();

        /**
         * 单击事件
         */
        void onSingleTap();

        /**
         * 双击事件
         */
        void onDoubleTap();
    }

    /**
     * 设置手势监听事件
     *
     * @param gestureListener 手势监听事件
     */
    public void setOnGestureListener(GestureListener gestureListener) {
        mOutGestureListener = gestureListener;
    }

    @Override
    public void reset() {
        mHideType = null;
    }

    @Override
    public void show() {
        if (mHideType == HideType.End) {
            //如果是由于错误引起的隐藏，那就不能再展现了
            VcPlayerLog.d(TAG, "show END");
        } else {
            VcPlayerLog.d(TAG, "show ");

            mIsOnlySingleTapEnable = false;
            setVisibility(VISIBLE);
        }
    }

    @Override
    public void hide(HideType hideType) {
        if (mHideType != HideType.End) {
            mHideType = hideType;

            mIsOnlySingleTapEnable = true;
            setVisibility(VISIBLE);
        }else{
            setVisibility(GONE);
        }
    }

    @Override
    public void setScreenModeStatus(CicadaScreenMode mode) {

    }

}
