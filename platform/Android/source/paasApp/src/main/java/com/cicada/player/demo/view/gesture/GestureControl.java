package com.cicada.player.demo.view.gesture;

import android.app.Activity;
import android.view.GestureDetector;
import android.view.GestureDetector.OnGestureListener;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;

import com.cicada.player.demo.util.ScreenUtils;

/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 播放控手势控制。通过对view的GestureDetector事件做监听，判断水平滑动还是垂直滑动。
 * 最后的结果通过{@link GestureView.GestureListener}返回出去。
 * 主要在{@link GestureView}中使用到此类。
 */
public class GestureControl {

    private static final String TAG = GestureControl.class.getSimpleName();

    /**
     * 播放控制层
     **/
    private View mGesturebleView;

    /**
     * 是否允许触摸 //TODO 可以删掉
     */
    private boolean isGestureEnable = true;
    //是否水平
    private boolean isInHorizenalGesture = false;
    //是否右边垂直
    private boolean isInRightGesture = false;
    //是否左边垂直
    private boolean isInLeftGesture = false;

    //手势决定器
    private GestureDetector mGestureDetector;
    //手势监听
    private GestureView.GestureListener mGestureListener;

    /**
     * @param gestureView 播放控制层
     */
    public GestureControl( View gestureView) {

        this.mGesturebleView = gestureView;
        init();
    }

    private void init() {

        mGestureDetector = new GestureDetector(mGesturebleView.getContext().getApplicationContext(), mOnGestureListener);
        mGesturebleView.setOnTouchListener(new OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                switch (event.getAction()) {
                    case MotionEvent.ACTION_UP:
                    case MotionEvent.ACTION_CANCEL:
//对结束事件的监听
                        if (mGestureListener != null) {
                            mGestureListener.onGestureEnd();
                        }

                        isInLeftGesture = false;
                        isInRightGesture = false;
                        isInHorizenalGesture = false;
                        break;

                    default:
                        break;
                }
//其他的事件交给GestureDetector。
                return mGestureDetector.onTouchEvent(event);

            }
        });

        //GestureDetector增加双击事件的监听。。里面包含了单击事件
        mGestureDetector.setOnDoubleTapListener(new GestureDetector.OnDoubleTapListener() {
            @Override
            public boolean onSingleTapConfirmed(MotionEvent e) {
                //			处理点击事件
                if (mGestureListener != null) {
                    mGestureListener.onSingleTap();
                }
                return false;
            }

            @Override
            public boolean onDoubleTap(MotionEvent e) {
                return false;
            }

            @Override
            public boolean onDoubleTapEvent(MotionEvent e) {
                if (mGestureListener != null) {
                    mGestureListener.onDoubleTap();
                }
                return false;
            }
        });
    }


    /**
     * 开启关闭手势控制。
     * @param enable  开启
     */
    void enableGesture(boolean enable) {
        this.isGestureEnable = enable;
    }

    /**
     * 设置手势监听事件
     * @param mGestureListener 手势监听事件
     */
    void setOnGestureControlListener(GestureView.GestureListener mGestureListener) {
        this.mGestureListener = mGestureListener;
    }

    /**
     * 绑定到GestureDetector的。
     */
    private final OnGestureListener mOnGestureListener = new OnGestureListener() {
        private float mXDown;

        @Override
        public boolean onSingleTapUp(MotionEvent e) {
            return false;
        }

        @Override
        public void onShowPress(MotionEvent e) {
        }

        @Override
        public boolean onScroll(MotionEvent e1, MotionEvent e2, float distanceX, float distanceY) {
            //如果关闭了手势。则不处理。
            if (!isGestureEnable || e1 == null || e2 == null) {
                return false;
            }

            if (Math.abs(distanceX) > Math.abs(distanceY)) {
                //水平滑动
                if (isInLeftGesture || isInRightGesture) {
                    //此前已经是竖直滑动了，不管
                } else {
                    isInHorizenalGesture = true;
                }

            } else {
                //垂直滑动
                if (isInHorizenalGesture) {
                } else {

                }
            }

            if (isInHorizenalGesture) {
                if (mGestureListener != null) {
                    mGestureListener.onHorizontalDistance(e1.getX(), e2.getX());
                }
            } else {
                if (ScreenUtils.isInLeft((Activity) mGesturebleView.getContext(), (int) mXDown)) {
                    isInLeftGesture = true;
                    if (mGestureListener != null) {
                        mGestureListener.onLeftVerticalDistance(e1.getY(), e2.getY());
                    }
                } else if (ScreenUtils.isInRight((Activity) mGesturebleView.getContext(), (int) mXDown)) {
                    isInRightGesture = true;
                    if (mGestureListener != null) {
                        mGestureListener.onRightVerticalDistance(e1.getY(), e2.getY());
                    }
                }
            }
            return true;
        }

        @Override
        public void onLongPress(MotionEvent e) {

        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            return false;
        }

        @Override
        public boolean onDown(MotionEvent e) {
            this.mXDown = e.getX();
            return true;
        }

    };


}
