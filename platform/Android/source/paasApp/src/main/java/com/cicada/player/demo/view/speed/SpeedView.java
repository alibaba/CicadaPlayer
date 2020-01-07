package com.cicada.player.demo.view.speed;

import android.content.Context;
import android.support.annotation.Nullable;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.view.ViewTreeObserver;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.widget.RadioButton;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cicada.player.demo.R;
import com.cicada.player.demo.listener.LockPortraitListener;
import com.cicada.player.demo.bean.CicadaScreenMode;
import com.cicada.player.demo.view.CicadaVodPlayerView;
import com.cicada.player.demo.util.VcPlayerLog;

/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 倍速播放界面。用于控制倍速。
 * 在{@link CicadaVodPlayerView}中使用。
 */

public class SpeedView extends RelativeLayout {

    private static final String TAG = SpeedView.class.getSimpleName();

    private SpeedValue mSpeedValue;

    private View mMainSpeedView;
    //显示动画
    private Animation showAnim;
    //隐藏动画
    private Animation hideAnim;
    //动画是否结束
    private boolean animEnd = true;

    // 正常倍速
    private RadioButton mNormalBtn;
    //1.25倍速
    private RadioButton mOneQrtTimeBtn;
    //1.5倍速
    private RadioButton mOneHalfTimeBtn;
    //2倍速
    private RadioButton mTwoTimeBtn;

    //切换结果的提示
    private TextView mSpeedTip;
    //屏幕模式
    private CicadaScreenMode mScreenMode;
    //倍速选择事件
    private OnSpeedClickListener mOnSpeedClickListener = null;
    //倍速是否变化
    private boolean mSpeedChanged = false;
    //选中的倍速的指示点的方块
    private int mSpeedDrawable = R.mipmap.cicada_speed_dot_blue;
    //选中的倍速的指示点的文字
    private int mSpeedTextColor = R.color.cicada_blue;

    public SpeedView(Context context) {
        super(context);
        init();
    }

    public SpeedView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }


    public SpeedView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        //初始化布局
        LayoutInflater.from(getContext()).inflate(R.layout.cicada_view_speed, this, true);
        mMainSpeedView = findViewById(R.id.speed_view);
        mMainSpeedView.setVisibility(INVISIBLE);

        //找出控件
        mOneQrtTimeBtn = (RadioButton) findViewById(R.id.one_quartern);
        mNormalBtn = (RadioButton) findViewById(R.id.normal);
        mOneHalfTimeBtn = (RadioButton) findViewById(R.id.one_half);
        mTwoTimeBtn = (RadioButton) findViewById(R.id.two);

        mSpeedTip = (TextView) findViewById(R.id.speed_tip);
        mSpeedTip.setVisibility(INVISIBLE);

        //对每个倍速项做点击监听
        mOneQrtTimeBtn.setOnClickListener(mClickListener);
        mNormalBtn.setOnClickListener(mClickListener);
        mOneHalfTimeBtn.setOnClickListener(mClickListener);
        mTwoTimeBtn.setOnClickListener(mClickListener);

        //倍速view使用到的动画
        showAnim = AnimationUtils.loadAnimation(getContext(), R.anim.view_speed_show);
        hideAnim = AnimationUtils.loadAnimation(getContext(), R.anim.view_speed_hide);
        showAnim.setAnimationListener(new Animation.AnimationListener() {
            @Override
            public void onAnimationStart(Animation animation) {
                //显示动画开始的时候，将倍速view显示出来
                animEnd = false;
                mMainSpeedView.setVisibility(VISIBLE);
            }

            @Override
            public void onAnimationEnd(Animation animation) {
                animEnd = true;
            }

            @Override
            public void onAnimationRepeat(Animation animation) {

            }
        });
        hideAnim.setAnimationListener(new Animation.AnimationListener() {
            @Override
            public void onAnimationStart(Animation animation) {
                animEnd = false;
            }

            @Override
            public void onAnimationEnd(Animation animation) {

                //隐藏动画结束的时候，将倍速view隐藏掉
                mMainSpeedView.setVisibility(INVISIBLE);
                if (mOnSpeedClickListener != null) {
                    mOnSpeedClickListener.onHide();
                }

                //如果倍速有变化，会提示倍速变化的消息
                if (mSpeedChanged) {
                    String times = "";
                    if (mSpeedValue == SpeedValue.OneQuartern) {
                        times = getResources().getString(R.string.cicada_speed_optf_times);
                    } else if (mSpeedValue == SpeedValue.Normal) {
                        times = getResources().getString(R.string.cicada_speed_one_times);
                    } else if (mSpeedValue == SpeedValue.OneHalf) {
                        times = getResources().getString(R.string.cicada_speed_opt_times);
                    } else if (mSpeedValue == SpeedValue.Twice) {
                        times = getResources().getString(R.string.cicada_speed_twice_times);
                    }
                    String tips = getContext().getString(R.string.cicada_speed_tips, times);
                    mSpeedTip.setText(tips);
                    mSpeedTip.setVisibility(VISIBLE);
                    mSpeedTip.postDelayed(new Runnable() {
                        @Override
                        public void run() {
                            mSpeedTip.setVisibility(INVISIBLE);
                        }
                    }, 1000);
                }
                animEnd = true;
            }

            @Override
            public void onAnimationRepeat(Animation animation) {

            }
        });

        setSpeed(SpeedValue.Normal);
//监听view的Layout事件
        getViewTreeObserver().addOnGlobalLayoutListener(new MyLayoutListener());
    }


    /**
     * 设置主题
     *
     * @param theme 支持的主题
     */
//    @Override
//    public void setTheme(AliyunVodPlayerView.Theme theme) {
//
//        mSpeedDrawable = R.drawable.cicada_speed_dot_blue;
//        mSpeedTextColor = R.color.alivc_blue;
//        //根据主题变化对应的颜色
//        if (theme == AliyunVodPlayerView.Theme.Blue) {
//            mSpeedDrawable = R.drawable.cicada_speed_dot_blue;
//            mSpeedTextColor = R.color.alivc_blue;
//        } else if (theme == AliyunVodPlayerView.Theme.Green) {
//            mSpeedDrawable = R.drawable.alivc_speed_dot_green;
//            mSpeedTextColor = R.color.alivc_green;
//        } else if (theme == AliyunVodPlayerView.Theme.Orange) {
//            mSpeedDrawable = R.drawable.alivc_speed_dot_orange;
//            mSpeedTextColor = R.color.alivc_orange;
//        } else if (theme == AliyunVodPlayerView.Theme.Red) {
//            mSpeedDrawable = R.drawable.alivc_speed_dot_red;
//            mSpeedTextColor = R.color.alivc_red;
//        }
//
//        updateBtnTheme();
//    }

    /**
     * 更新按钮的颜色之类的
     */
    private void setRadioButtonTheme(RadioButton button) {
        if (button.isChecked()) {
            button.setCompoundDrawablesWithIntrinsicBounds(0, mSpeedDrawable, 0, 0);
            button.setTextColor(getResources().getColor(mSpeedTextColor));
        } else {
            button.setCompoundDrawablesWithIntrinsicBounds(0, 0, 0, 0);
            button.setTextColor(getResources().getColor(R.color.cicada_white));
        }
    }

    private class MyLayoutListener implements ViewTreeObserver.OnGlobalLayoutListener {
        private CicadaScreenMode lastLayoutMode = null;

        @Override
        public void onGlobalLayout() {
            if (mMainSpeedView.getVisibility() == VISIBLE) {

                //防止重复设置
                if (lastLayoutMode == mScreenMode) {
                    return;
                }

                setScreenMode(mScreenMode);
                lastLayoutMode = mScreenMode;
            }
        }
    }

    private OnClickListener mClickListener = new OnClickListener() {
        @Override
        public void onClick(View view) {
            if (mOnSpeedClickListener == null) {
                return;
            }

            if (view == mNormalBtn) {
                mOnSpeedClickListener.onSpeedClick(SpeedValue.Normal);
            } else if (view == mOneQrtTimeBtn) {
                mOnSpeedClickListener.onSpeedClick(SpeedValue.OneQuartern);
            } else if (view == mOneHalfTimeBtn) {
                mOnSpeedClickListener.onSpeedClick(SpeedValue.OneHalf);
            } else if (view == mTwoTimeBtn) {
                mOnSpeedClickListener.onSpeedClick(SpeedValue.Twice);
            }
        }

    };

    /**
     * 设置倍速点击事件
     *
     * @param l
     */
    public void setOnSpeedClickListener(OnSpeedClickListener l) {
        mOnSpeedClickListener = l;
    }

    /**
     * 设置当前屏幕模式。不同的模式，speedView的大小不一样
     *
     * @param screenMode
     */
    public void setScreenMode(CicadaScreenMode screenMode) {
        ViewGroup.LayoutParams speedViewParam = mMainSpeedView.getLayoutParams();


        if (screenMode == CicadaScreenMode.Small) {
            //小屏的时候，是铺满整个播放器的
            speedViewParam.width = getWidth();
            speedViewParam.height = getHeight();
        } else if (screenMode == CicadaScreenMode.Full) {
            //如果是全屏的，就显示一半
            CicadaVodPlayerView parentView = (CicadaVodPlayerView) getParent();
            LockPortraitListener lockPortraitListener = parentView.getLockPortraitMode();
            if (lockPortraitListener == null) {
                //没有设置这个监听，说明不是固定模式，按正常的界面显示就OK
                speedViewParam.width = getWidth() / 2;
            } else {
                speedViewParam.width = getWidth();
            }
            speedViewParam.height = getHeight();
        }

        VcPlayerLog.d(TAG, "setScreenModeStatus screenMode = " + screenMode.name() + " , width = " + speedViewParam.width + " , height = " + speedViewParam.height);
        mScreenMode = screenMode;
        mMainSpeedView.setLayoutParams(speedViewParam);
    }

    /**
     * 倍速监听
     */
    public interface OnSpeedClickListener {
        /**
         * 选中某个倍速
         *
         * @param value 倍速值
         */
        void onSpeedClick(SpeedValue value);

        /**
         * 倍速界面隐藏
         */
        void onHide();
    }

    /**
     * 倍速值
     */
    public static enum SpeedValue {
        /**
         * 正常倍速
         */
        Normal,
        /**
         * 1.25倍速
         */
        OneQuartern,
        /**
         * 1.5倍速
         */
        OneHalf,
        /**
         * 2倍速
         */
        Twice
    }


    /**
     * 设置显示的倍速
     *
     * @param speedValue 倍速值
     */
    public void setSpeed(SpeedValue speedValue) {
        if (speedValue == null) {
            return;
        }

        if (mSpeedValue != speedValue) {
            mSpeedValue = speedValue;
            mSpeedChanged = true;
            updateSpeedCheck();
        } else {
            mSpeedChanged = false;
        }

        hide();

    }

    /**
     * 更新倍速选项的状态
     */
    private void updateSpeedCheck() {
        mOneQrtTimeBtn.setChecked(mSpeedValue == SpeedValue.OneQuartern);
        mNormalBtn.setChecked(mSpeedValue == SpeedValue.Normal);
        mOneHalfTimeBtn.setChecked(mSpeedValue == SpeedValue.OneHalf);
        mTwoTimeBtn.setChecked(mSpeedValue == SpeedValue.Twice);

        updateBtnTheme();
    }

    /**
     * 更新选项的Theme
     */
    private void updateBtnTheme() {
        setRadioButtonTheme(mNormalBtn);
        setRadioButtonTheme(mOneQrtTimeBtn);
        setRadioButtonTheme(mOneHalfTimeBtn);
        setRadioButtonTheme(mTwoTimeBtn);
    }

    /**
     * 显示倍速view
     *
     * @param screenMode 屏幕模式
     */
    public void show(CicadaScreenMode screenMode) {

        setScreenMode(screenMode);

        mMainSpeedView.startAnimation(showAnim);

    }

    /**
     * 隐藏
     */
    private void hide() {
        if (mMainSpeedView.getVisibility() == VISIBLE) {
            mMainSpeedView.startAnimation(hideAnim);
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        //动画没有结束的时候，触摸是没有效果的
        if (mMainSpeedView.getVisibility() == VISIBLE && animEnd) {
            hide();
            return true;
        }

        return super.onTouchEvent(event);
    }


}
