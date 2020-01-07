package com.cicada.player.demo.view;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RelativeLayout;

import com.cicada.player.demo.view.tipsview.ErrorView;
import com.cicada.player.demo.view.tipsview.LoadingView;
import com.cicada.player.demo.view.tipsview.NetChangeView;
import com.cicada.player.demo.view.tipsview.ReplayView;
import com.cicada.player.demo.util.VcPlayerLog;

/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 提示对话框的管理器。
 * 用于管理{@link ErrorView} ，{@link LoadingView} ，{@link NetChangeView} , {@link ReplayView}等view的显示/隐藏等。
 */

public class TipsView extends RelativeLayout {

    private static final String TAG = TipsView.class.getSimpleName();
    //错误码
    private String mErrorCode;
    //错误提示
    private ErrorView mErrorView = null;
    //重试提示
    private ReplayView mReplayView = null;
    //缓冲加载提示
    private LoadingView mNetLoadingView = null;
    //网络变化提示
    private NetChangeView mNetChangeView = null;
    //重新加载提示
    private NetChangeView mNetWorkRetryView = null;
    //网络请求加载提示
    private LoadingView mBufferLoadingView = null;
    //提示点击事件
    private OnTipClickListener mOnTipClickListener = null;
    //当前的主题
//    private AliyunVodPlayerView.Theme mCurrentTheme;

    //网络变化监听事件。
    private NetChangeView.OnNetChangeClickListener onNetChangeClickListener = new NetChangeView.OnNetChangeClickListener() {
        @Override
        public void onContinuePlay() {
            if (mOnTipClickListener != null) {
                mOnTipClickListener.onContinuePlay();
            }
        }

        @Override
        public void onStopPlay() {
            if (mOnTipClickListener != null) {
                mOnTipClickListener.onStopPlay();
            }
        }
    };
    //错误提示的重试点击事件
    private ErrorView.OnRetryClickListener onRetryClickListener = new ErrorView.OnRetryClickListener() {
        @Override
        public void onRetryClick() {
            if (mOnTipClickListener != null) {
                mOnTipClickListener.onRetryPlay();
            }
        }
    };

    //重播点击事件
    private ReplayView.OnReplayClickListener onReplayClickListener = new ReplayView.OnReplayClickListener() {
        @Override
        public void onReplay() {


            if (mOnTipClickListener != null) {
                mOnTipClickListener.onReplay();
            }
        }
    };

    public TipsView(Context context) {
        super(context);
    }

    public TipsView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public TipsView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }


    /**
     * 显示网络变化提示
     */
    public void showNetChangeTipView() {
        if (mNetChangeView == null) {
            mNetChangeView = new NetChangeView(getContext());
            mNetChangeView.setOnNetChangeClickListener(onNetChangeClickListener);
            addSubView(mNetChangeView);
        }

        if (mErrorView != null && mErrorView.getVisibility() == VISIBLE) {
            //显示错误对话框了，那么网络切换的对话框就不显示了。
            //都出错了，还显示网络切换，没有意义
        } else {
            mNetChangeView.setVisibility(VISIBLE);
        }

    }
    /**
     * 显示错误提示
     *
     * @param errorCode  错误码
     * @param errorEvent 错误事件
     * @param errorMsg   错误消息
     */
    public void showErrorTipView(String errorCode, String errorEvent, String errorMsg) {
        if (mErrorView == null) {
            mErrorView = new ErrorView(getContext());
            mErrorView.setOnRetryClickListener(onRetryClickListener);
            addSubView(mErrorView);
        }

        //出现错误了，先把其他的提示对话框关闭掉。防止同时显示多个对话框。
        hideAll();

        mErrorCode = errorCode;
        mErrorView.updateTips(errorCode, errorEvent, errorMsg);
        mErrorView.setVisibility(VISIBLE);


        VcPlayerLog.d(TAG, " errorCode = " + mErrorCode);
    }

    /**
     * 显示重播view
     */
    public void showReplayTipView() {
        if (mReplayView == null) {
            mReplayView = new ReplayView(getContext());
            mReplayView.setOnReplayClickListener(onReplayClickListener);
            addSubView(mReplayView);
        }

        if (mReplayView.getVisibility() != VISIBLE) {
            mReplayView.setVisibility(VISIBLE);
        }
    }


    /**
     * 显示缓冲加载view
     */
    public void showBufferLoadingTipView() {
        if (mBufferLoadingView == null) {
            mBufferLoadingView = new LoadingView(getContext());
            addSubView(mBufferLoadingView);
        }
        if (mBufferLoadingView.getVisibility() != VISIBLE) {
            mBufferLoadingView.updateLoadingPercent(0);
            mBufferLoadingView.setVisibility(VISIBLE);
        }
    }

    /**
     * 更新缓冲加载的进度
     *
     * @param percent 进度百分比
     */
    public void updateLoadingPercent(int percent) {
        showBufferLoadingTipView();
        mBufferLoadingView.updateLoadingPercent(percent);
    }

    /**
     * 显示网络加载view
     */
    public void showNetLoadingTipView() {
        if (mNetLoadingView == null) {
            mNetLoadingView = new LoadingView(getContext());
            mNetLoadingView.setOnlyLoading();
            addSubView(mNetLoadingView);
        }

        if (mNetLoadingView.getVisibility() != VISIBLE) {
            mNetLoadingView.setVisibility(VISIBLE);
        }
    }


    /**
     * 把新增的view添加进来，居中添加
     *
     * @param subView 子view
     */
    private void addSubView(View subView) {
        LayoutParams params = new LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
        params.addRule(CENTER_IN_PARENT);
        addView(subView, params);

        //同时需要更新新加的view的主题
//        if (subView instanceof ITheme) {
//            ((ITheme) subView).setTheme(mCurrentTheme);
//        }
    }


    /**
     * 隐藏所有的tip
     */
    public void hideAll() {
        hideNetChangeTipView();
        hideErrorTipView();
        hideReplayTipView();
        hideBufferLoadingTipView();
        hideNetLoadingTipView();
    }

    /**
     * 隐藏缓冲加载的tip
     */
    public void hideBufferLoadingTipView() {
        if (mBufferLoadingView != null && mBufferLoadingView.getVisibility() == VISIBLE) {
            mBufferLoadingView.setVisibility(INVISIBLE);
        }
    }

    /**
     * 隐藏网络加载的tip
     */
    public void hideNetLoadingTipView() {
        if (mNetLoadingView != null && mNetLoadingView.getVisibility() == VISIBLE) {
            mNetLoadingView.setVisibility(INVISIBLE);
        }
    }

    /**
     * 隐藏重播的tip
     */
    public void hideReplayTipView() {
        if (mReplayView != null && mReplayView.getVisibility() == VISIBLE) {
            mReplayView.setVisibility(INVISIBLE);
        }
    }

    /**
     * 隐藏网络变化的tip
     */
    public void hideNetChangeTipView() {
        if (mNetChangeView != null && mNetChangeView.getVisibility() == VISIBLE) {
            mNetChangeView.setVisibility(INVISIBLE);
        }
    }

    /**
     * 隐藏错误的tip
     */
    public void hideErrorTipView() {
        if (mErrorView != null && mErrorView.getVisibility() == VISIBLE) {
            mErrorView.setVisibility(INVISIBLE);
        }
    }

    /**
     * 错误的tip是否在显示，如果在显示的话，其他的tip就不提示了。
     *
     * @return true：是
     */
    public boolean isErrorShow() {
        if (mErrorView != null) {
            return mErrorView.getVisibility() == VISIBLE;
        } else {
            return false;
        }
    }

    /**
     * 隐藏网络错误tip
     */
    public void hideNetErrorTipView() {
        if (mErrorView != null && mErrorView.getVisibility() == VISIBLE) {
            mErrorView.setVisibility(INVISIBLE);
        }
    }
//    @Override
//    public void setTheme(AliyunVodPlayerView.Theme theme) {
//
//        mCurrentTheme = theme;
//        //判断子view是不是实现了ITheme的接口，从而达到更新主题的目的
//        int childCount = getChildCount();
//        for (int i = 0; i < childCount; i++) {
//            View child = getChildAt(i);
//            if (child instanceof ITheme) {
//                ((ITheme) child).setTheme(theme);
//            }
//        }
//    }

    /**
     * 提示view中的点击操作
     */
    public interface OnTipClickListener {
        /**
         * 继续播放
         */
        void onContinuePlay();

        /**
         * 停止播放
         */
        void onStopPlay();

        /**
         * 重试播放
         */
        void onRetryPlay();

        /**
         * 重播
         */
        void onReplay();
    }

    /**
     * 设置提示view中的点击操作 监听
     *
     * @param l 监听事件
     */
    public void setOnTipClickListener(OnTipClickListener l) {
        mOnTipClickListener = l;
    }
}
