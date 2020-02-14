package com.cicada.player.demo.view;

import android.app.Activity;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.Bitmap;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.cicada.player.CicadaPlayer;
import com.cicada.player.CicadaPlayerFactory;
import com.cicada.player.bean.ErrorInfo;
import com.cicada.player.bean.InfoBean;
import com.cicada.player.bean.InfoCode;
import com.cicada.player.demo.R;
import com.cicada.player.demo.bean.CicadaScreenMode;
import com.cicada.player.demo.listener.LockPortraitListener;
import com.cicada.player.demo.listener.ViewAction;
import com.cicada.player.demo.util.BrightnessUtil;
import com.cicada.player.demo.util.DensityUtils;
import com.cicada.player.demo.util.OrientationWatchDog;
import com.cicada.player.demo.util.ScreenUtils;
import com.cicada.player.demo.util.SharedPreferenceUtils;
import com.cicada.player.demo.view.control.ControlView;
import com.cicada.player.demo.view.gesture.GestureView;
import com.cicada.player.demo.view.guide.GuideView;
import com.cicada.player.demo.view.quality.QualityView;
import com.cicada.player.demo.view.speed.SpeedView;
import com.cicada.player.demo.view.subtitle.SubtitleView;
import com.cicada.player.demo.view.thumbnail.ThumbnailView;
import com.cicada.player.nativeclass.CacheConfig;
import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.PlayerConfig;
import com.cicada.player.nativeclass.TrackInfo;
import com.cicada.player.demo.util.NetWatchdog;
import com.cicada.player.demo.util.VcPlayerLog;
import com.cicada.player.utils.Logger;

import java.io.File;
import java.lang.ref.WeakReference;

import static com.cicada.player.demo.view.subtitle.LocationStyle.Location_CenterH;
import static com.cicada.player.demo.view.subtitle.LocationStyle.Location_Top;

/**
 * UI播放器的主要实现类。
 * 通过ITheme控制各个界面的主题色。
 * 通过各种view的组合实现UI的界面。这些view包括：
 * 用户手势操作的{@link GestureView}
 * 控制播放，显示信息的{@link ControlView}
 * 显示清晰度列表的{@link QualityView}
 * 倍速选择界面{@link SpeedView}
 * 用户使用引导页面{@link GuideView}
 * 用户提示页面{@link TipsView}
 * 以及封面等。
 * view 的初始化是在{@link #initVideoView}方法中实现的。
 * 然后是对各个view添加监听方法，处理对应的操作，从而实现与播放器的共同操作
 */
public class CicadaVodPlayerView extends RelativeLayout {

    private static final String TAG = CicadaVodPlayerView.class.getSimpleName();

    /**
     * 视频画面
     */
    private SurfaceView mSurfaceView;
    /**
     * 手势操作view
     */
    private GestureView mGestureView;
    /**
     * 皮肤view
     */
    private ControlView mControlView;
    /**
     * 清晰度view
     */
    private QualityView mQualityView;
    /**
     * 倍速选择view
     */
    private SpeedView mSpeedView;
    /**
     * 引导页view
     */
    private GuideView mGuideView;
    /**
     * 封面view
     */
    private ImageView mCoverView;
    /**
     * 播放器
     */
    private CicadaPlayer mCicadaVodPlayer;
    /**
     * 手势对话框控制
     */
    private GestureDialogManager mGestureDialogManager;
    /**
     * 网络状态监听
     */
    private NetWatchdog mNetWatchdog;
    /**
     * 屏幕方向监听
     */
    private OrientationWatchDog mOrientationWatchDog;
    /**
     * Tips view
     */
    private TipsView mTipsView;
    /**
     * 锁定竖屏
     */
    private LockPortraitListener mLockPortraitListener = null;
    /**
     * 是否锁定全屏
     */
    private boolean mIsFullScreenLocked = false;
    /**
     * 当前屏幕模式
     */
    private CicadaScreenMode mCurrentScreenMode = CicadaScreenMode.Small;
    /**
     * 是不是在seek中
     */
    private boolean inSeek = false;
    /**
     * 播放是否完成
     */
    private boolean isCompleted = false;
    /**
     * 是否是重播
     */
    private boolean isReplay = false;
    /**
     * 媒体信息
     */
    private MediaInfoExt mAliyunMediaInfo;
    /**
     * 整体缓冲进度
     */
    private int mCurrentBufferPercentage = 0;
    /**
     * 字幕
     */
    private SubtitleView subtitleView;
    /**
     * 缩略图View
     */
    private ThumbnailView mThumbnailView;

    private String mUrlSource = null;

    /**
     * 是否需要开启重试机制,如果是wifi或者无网络的情况下需要进行重试
     */
    private boolean mNeedToRetry = false;
    /**
     * 重连次数
     */
    private int mRetryTime = 3;

    /**
     * 缩略图帮助类
     */
//    private ThumbnailHelper mThumbnailHelper;

    /**
     * 上一个外挂字幕的trackIndex
     */
    private int mPreviewTrackIndex;

    /**
     * 对外的各种事件监听
     */
    private CicadaPlayer.OnInfoListener mOutInfoListener = null;
    private CicadaPlayer.OnErrorListener mOutErrorListener = null;
    private CicadaPlayer.OnSnapShotListener mSnapShotListener = null;
    private CicadaPlayer.OnPreparedListener mOutPreparedListener = null;
    //    private CicadaPlayer.OnThumbnailListener mOnThumbnailListener = null;
    private CicadaPlayer.OnCompletionListener mOutCompletionListener = null;
    private CicadaPlayer.OnSeekCompleteListener mOuterSeekCompleteListener = null;
    private CicadaPlayer.OnTrackChangedListener mOutTrackChangedListener = null;
    private CicadaPlayer.OnSubtitleDisplayListener mOutSubtitleDisplayListener = null;
    private long startSeekTime;
    /**
     * seek模式,默认为非精准模式
     */
    private CicadaPlayer.SeekMode mSeekMode = CicadaPlayer.SeekMode.Inaccurate;


    public CicadaVodPlayerView(Context context) {
        super(context);
        initVideoView();
    }

    public CicadaVodPlayerView(Context context, AttributeSet attrs) {
        super(context, attrs);
        initVideoView();
    }

    public CicadaVodPlayerView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        initVideoView();
    }

    /**
     * 初始化view
     */
    private void initVideoView() {
        //初始化播放用的surfaceView
        initSurfaceView();
        //初始化播放器
        initCicadaPlayer();
        //初始化封面
        initCoverView();
        //初始化字幕();
        initSubTitle();
        //初始化手势view
        initGestureView();
        //初始化清晰度view
//        initQualityView();
        //初始化控制栏
        initControlView();
        //初始化倍速view
        initSpeedView();
        //初始化指引view
        initGuideView();
        //初始化提示view
        initTipsView();
        //初始化网络监听器
        initNetWatchdog();
        //初始化屏幕方向监听
        initOrientationWatchdog();
        //初始化手势对话框控制
        initGestureDialogManager();
        //先隐藏手势和控制栏，防止在没有prepare的时候做操作。
        hideGestureAndControlViews();
        //初始化缩略图
        initThumbnailView();
    }

    /**
     * 隐藏手势和控制栏
     */
    private void hideGestureAndControlViews() {
        if (mGestureView != null) {
            mGestureView.hide(ViewAction.HideType.Normal);
        }
//        if (mControlView != null) {
//            mControlView.hide(ViewAction.HideType.Normal);
//        }
    }


    /**
     * 初始化网络监听
     */
    private void initNetWatchdog() {
        Context context = getContext();
        mNetWatchdog = new NetWatchdog(context);
        mNetWatchdog.setNetChangeListener(new MyNetChangeListener(this));
    }

    /**
     * 初始化缩略图
     */
    private void initThumbnailView() {
        mThumbnailView = new ThumbnailView(getContext());
        addSubViewByCenter(mThumbnailView);
    }


    private void onWifiTo4G() {
        VcPlayerLog.d(TAG, "onWifiTo4G");
        mNeedToRetry = false;
        if (mUrlSource != null) {
            if (TextUtils.isEmpty(mUrlSource)) {
                return;
            }
            File file = new File(mUrlSource);
            if (file.exists()) {
                //本地的文件，不是网络文件，就没有必要提示4G切换了。
                return;
            }
        }

        //如果已经显示错误了，那么就不用显示网络变化的提示了。
        if (mTipsView.isErrorShow()) {
            return;
        }

        //wifi变成4G，先暂停播放
        pause();

        //隐藏其他的动作,防止点击界面去进行其他操作
        mGestureView.hide(ControlView.HideType.Normal);
//        mControlView.hide(ControlView.HideType.Normal);

        //显示网络变化的提示
        if (mTipsView != null) {
            mTipsView.showNetChangeTipView();
            mGestureDialogManager.dismissAllDialog();
        }
    }

    private void on4GToWifi() {
        VcPlayerLog.d(TAG, "on4GToWifi");
        mNeedToRetry = true;
        //如果已经显示错误了，那么就不用显示网络变化的提示了。
        if (mTipsView.isErrorShow()) {
            return;
        }

        //隐藏网络变化的提示
        if (mTipsView != null) {
            mTipsView.hideNetErrorTipView();
        }
        if (mRetryTime <= 0) {
            //切换网络的时候重新进行reload尝试
            mRetryTime = 3;
            networkRetry();
        }
    }

    /**
     * 进行网络重连
     */
    private void networkRetry() {
        if (mRetryTime > 0) {
            if (mNeedToRetry) {
                mCicadaVodPlayer.reload();
                mRetryTime--;
            }
        } else {
            Toast.makeText(getContext(), R.string.cicada_tip_network_reconnect_failed, Toast.LENGTH_SHORT).show();
        }
    }

    private void onNetDisconnected() {
        VcPlayerLog.d(TAG, "onNetDisconnected");
        mNeedToRetry = true;
        //网络断开。
        // NOTE： 由于安卓这块网络切换的时候，有时候也会先报断开。所以这个回调是不准确的。
    }

    public void setDataSource(String urlDataSource) {


        if (mCicadaVodPlayer == null) {
            return;
        }

        clearAllSource();
        reset();

        mUrlSource = urlDataSource;

        if (mControlView != null) {
            mControlView.setForceQuality(true);
        }

        String path = mUrlSource;
        if (TextUtils.isEmpty(path)) {
            path = "";
        }
        File file = new File(path);
        if (file.exists()) {
            //本地的文件，不是网络文件，就没有必要提示4G切换了。
            prepareLocalSource(mUrlSource);
            return;
        }
        prepareLocalSource(mUrlSource);

    }

    private void prepareLocalSource(String aliyunLocalSource) {
        if (mControlView != null) {
            mControlView.setForceQuality(true);
        }
        if (mControlView != null) {
            mControlView.setIsMtsSource(false);
        }
        if (mQualityView != null) {
            mQualityView.setIsMtsSource(false);
        }
        mCicadaVodPlayer.setDataSource(aliyunLocalSource);
    }

    private static class MyNetChangeListener implements NetWatchdog.NetChangeListener {

        private WeakReference<CicadaVodPlayerView> viewWeakReference;

        public MyNetChangeListener(CicadaVodPlayerView cicadaVodPlayerView) {
            viewWeakReference = new WeakReference<>(cicadaVodPlayerView);
        }

        @Override
        public void onWifiTo4G() {
            CicadaVodPlayerView cicadaVodPlayerView = viewWeakReference.get();
            if (cicadaVodPlayerView != null) {
                cicadaVodPlayerView.onWifiTo4G();
            }
        }

        @Override
        public void on4GToWifi() {
            CicadaVodPlayerView cicadaVodPlayerView = viewWeakReference.get();
            if (cicadaVodPlayerView != null) {
                cicadaVodPlayerView.on4GToWifi();
            }
        }

        @Override
        public void onNetDisconnected() {
            CicadaVodPlayerView cicadaVodPlayerView = viewWeakReference.get();
            if (cicadaVodPlayerView != null) {
                cicadaVodPlayerView.onNetDisconnected();
            }
        }
    }


    /**
     * 初始化屏幕方向旋转。用来监听屏幕方向。结果通过OrientationListener回调出去。
     */
    private void initOrientationWatchdog() {
        final Context context = getContext();
        mOrientationWatchDog = new OrientationWatchDog(context);
        mOrientationWatchDog.setOnOrientationListener(new InnerOrientationListener(this));
    }


    private static class InnerOrientationListener implements OrientationWatchDog.OnOrientationListener {

        private WeakReference<CicadaVodPlayerView> playerViewWeakReference;


        public InnerOrientationListener(CicadaVodPlayerView playerView) {
            playerViewWeakReference = new WeakReference<CicadaVodPlayerView>(playerView);
        }

        @Override
        public void changedToLandForwardScape(boolean fromPort) {
            CicadaVodPlayerView playerView = playerViewWeakReference.get();
            if (playerView != null) {
                playerView.changedToLandForwardScape(fromPort);
            }
        }

        @Override
        public void changedToLandReverseScape(boolean fromPort) {
            CicadaVodPlayerView playerView = playerViewWeakReference.get();
            if (playerView != null) {
                playerView.changedToLandReverseScape(fromPort);
            }
        }

        @Override
        public void changedToPortrait(boolean fromLand) {
            CicadaVodPlayerView playerView = playerViewWeakReference.get();
            if (playerView != null) {
                playerView.changedToPortrait(fromLand);
            }
        }
    }

    /**
     * 屏幕方向变为横屏。
     *
     * @param fromPort 是否从竖屏变过来
     */
    private void changedToLandForwardScape(boolean fromPort) {
        //如果不是从竖屏变过来，也就是一直是横屏的时候，就不用操作了
        if (!fromPort) {
            return;
        }
        changeScreenMode(CicadaScreenMode.Full, false);
    }

    /**
     * 屏幕方向变为横屏。
     *
     * @param fromPort 是否从竖屏变过来
     */
    private void changedToLandReverseScape(boolean fromPort) {
        //如果不是从竖屏变过来，也就是一直是横屏的时候，就不用操作了
        if (!fromPort) {
            return;
        }
        changeScreenMode(CicadaScreenMode.Full, true);
    }

    /**
     * 屏幕方向变为竖屏
     *
     * @param fromLand 是否从横屏转过来
     */
    private void changedToPortrait(boolean fromLand) {
        //屏幕转为竖屏
        if (mIsFullScreenLocked) {
            return;
        }

        if (mCurrentScreenMode == CicadaScreenMode.Full) {
            //全屏情况转到了竖屏
            if (getLockPortraitMode() == null) {
                //没有固定竖屏，就变化mode
                if (fromLand) {
                    changeScreenMode(CicadaScreenMode.Small, false);
                } else {
                    //如果没有转到过横屏，就不让他转了。防止竖屏的时候点横屏之后，又立即转回来的现象
                }
            } else {
                //固定竖屏了，竖屏还是竖屏，不用动
            }
        } else if (mCurrentScreenMode == CicadaScreenMode.Small) {
            //竖屏的情况转到了竖屏
        }
    }


    /**
     * 初始化手势的控制类
     */
    private void initGestureDialogManager() {
        Context context = getContext();
        if (context instanceof Activity) {
            mGestureDialogManager = new GestureDialogManager((Activity) context);
        }
    }

    private boolean continuePlay = false;

    /**
     * 初始化提示view
     */
    private void initTipsView() {

        mTipsView = new TipsView(getContext());
        //设置tip中的点击监听事件
        mTipsView.setOnTipClickListener(new TipsView.OnTipClickListener() {
            @Override
            public void onContinuePlay() {
                VcPlayerLog.d(TAG, "playerState = " + currentPlayState);
                //继续播放。如果没有prepare或者stop了，需要重新prepare
                mTipsView.hideAll();
                mNeedToRetry = true;
                if (mRetryTime <= 0) {
                    //切换网络的时候重新进行reload尝试
                    mRetryTime = 3;
                    networkRetry();
                }
                if (currentPlayState == CicadaPlayer.idle ||
                        currentPlayState == CicadaPlayer.stopped) {
                    ////4G进入播放的时候，
                    //continuePlay = true;
                    if (mUrlSource != null) {
                        setDataSource(mUrlSource);
                    }

                } else {
                    start();
                }

            }

            @Override
            public void onStopPlay() {
                // 结束播放
                mTipsView.hideAll();
                stop();

                Context context = getContext();
                if (context instanceof Activity) {
                    ((Activity) context).finish();
                }
            }

            @Override
            public void onRetryPlay() {
                //重试
                reTry();
            }

            @Override
            public void onReplay() {
                //重播
                mTipsView.hideReplayTipView();
                rePlay();
            }
        });
        addSubView(mTipsView);
    }

    /**
     * 重试播放，会从当前位置开始播放
     */
    public void reTry() {

        isCompleted = false;
        inSeek = false;

        int currentPosition = mControlView.getVideoPosition();
        VcPlayerLog.d(TAG, " currentPosition = " + currentPosition);

        if (mTipsView != null) {
            mTipsView.hideAll();
        }
        if (mControlView != null) {
            mControlView.reset();
            //防止被reset掉，下次还可以获取到这些值
            mControlView.setVideoPosition(currentPosition);
        }
        if (mGestureView != null) {
            mGestureView.reset();
        }

        if (mCicadaVodPlayer != null) {

            //显示网络加载的loading。。
            if (mTipsView != null) {
                mTipsView.showNetLoadingTipView();
                mGestureDialogManager.dismissAllDialog();
            }
            //seek到当前的位置再播放
            continuePlay = true;
            mCicadaVodPlayer.prepare();
        }

    }

    /**
     * 重播，将会从头开始播放
     */
    public void rePlay() {

        isCompleted = false;
        inSeek = false;

        if (mTipsView != null) {
            mTipsView.hideAll();
        }
        if (mControlView != null) {
            mControlView.reset();
        }
        if (mGestureView != null) {
            mGestureView.reset();
        }

        if (mCicadaVodPlayer != null) {
            //显示网络加载的loading。。
            if (mTipsView != null) {
                mTipsView.showNetLoadingTipView();
                mGestureDialogManager.dismissAllDialog();
            }
            //重播是从头开始播
            continuePlay = false;
            isReplay = true;
            mCicadaVodPlayer.prepare();
        }

    }

    /**
     * 重置。包括一些状态值，view的状态等
     */
    private void reset() {
        isCompleted = false;
        inSeek = false;

        if (mTipsView != null) {
            mTipsView.hideAll();
        }
        if (mControlView != null) {
            mControlView.reset();
        }
        if (mGestureView != null) {
            mGestureView.reset();
        }
        stop();
    }

    /**
     * 初始化封面
     */
    private void initCoverView() {
        mCoverView = new ImageView(getContext());
        //这个是为了给自动化测试用的id
        mCoverView.setId(R.id.custom_id_min);
        addSubView(mCoverView);
    }

    /**
     * 初始化字幕
     */
    private void initSubTitle() {
        subtitleView = new SubtitleView(getContext());
        SubtitleView.DefaultValueBuilder builder = new SubtitleView.DefaultValueBuilder();
        builder.setLocation(Location_Top | Location_CenterH);
        subtitleView.setDefaultValue(builder);
        subtitleView.setId(R.id.cicada_player_subtitle);
        addSubView(subtitleView);
    }

    /**
     * 初始化控制栏view
     */
    private void initControlView() {
        mControlView = new ControlView(getContext());
        addSubView(mControlView);

        //设置播放按钮点击
        mControlView.setOnPlayStateClickListener(new ControlView.OnPlayStateClickListener() {
            @Override
            public void onPlayStateClick() {
                switchPlayerState();
            }
        });
        //设置进度条的seek监听
        mControlView.setOnSeekListener(new ControlView.OnSeekListener() {
            @Override
            public void onSeekEnd(int position) {
                mControlView.setVideoPosition(position);

                if (isCompleted) {
                    //播放完成了，不能seek了
                    inSeek = false;
                } else {

                    //拖动结束后，开始seek
                    seekTo(position);

                    inSeek = true;

                    if (mThumbnailView != null) {
                        mThumbnailView.hideThumbnailView();
                    }
                }
            }

            @Override
            public void onSeekStart() {
                //拖动开始
                inSeek = true;
                if (mThumbnailView != null) {
                    mThumbnailView.showThumbnailView();
                    //根据屏幕大小调整缩略图的大小
                    ImageView thumbnailImageView = mThumbnailView.getThumbnailImageView();
                    if (thumbnailImageView != null) {
                        ViewGroup.LayoutParams layoutParams = thumbnailImageView.getLayoutParams();
                        layoutParams.width = (int) (ScreenUtils.getWidth(getContext()) / 3);
                        layoutParams.height = layoutParams.width / 2 - DensityUtils.px2dip(getContext(), 10);
                        thumbnailImageView.setLayoutParams(layoutParams);
                    }
                }
            }

            @Override
            public void onProgressChanged(int progress) {
//                if(mThumbnailHelper != null && mThumbnailPrepareSuccess){
//                    mThumbnailHelper.requestBitmapAtPosition(progress);
//                }
            }
        });

        //清晰度按钮点击
//        mControlView.setOnQualityBtnClickListener(new ControlView.OnQualityBtnClickListener() {
//
//            @Override
//            public void onQualityBtnClick(View v, List<TrackInfo> trackInfos, String currentQuality) {
//                //显示清晰度列表
//                mQualityView.setQuality(trackInfos, currentQuality);
//                mQualityView.showAtTop(v);
//            }
//
//            @Override
//            public void onHideQualityView() {
//                mQualityView.hide();
//            }
//        });
        //点击锁屏的按钮
        mControlView.setOnScreenLockClickListener(new ControlView.OnScreenLockClickListener() {
            @Override
            public void onClick() {
                lockScreen(!mIsFullScreenLocked);
            }
        });
        //点击全屏/小屏按钮
        mControlView.setOnScreenModeClickListener(new ControlView.OnScreenModeClickListener() {
            @Override
            public void onClick() {
                CicadaScreenMode targetMode;
                if (mCurrentScreenMode == CicadaScreenMode.Small) {
                    targetMode = CicadaScreenMode.Full;
                } else {
                    targetMode = CicadaScreenMode.Small;
                }
                changeScreenMode(targetMode, false);
            }
        });
        //点击了标题栏的返回按钮
        mControlView.setOnBackClickListener(new ControlView.OnBackClickListener() {
            @Override
            public void onClick() {

                if (mCurrentScreenMode == CicadaScreenMode.Full) {
                    //设置为小屏状态
                    changeScreenMode(CicadaScreenMode.Small, false);
                } else if (mCurrentScreenMode == CicadaScreenMode.Small) {
                    //小屏状态下，就结束活动
                    Context context = getContext();
                    if (context instanceof Activity) {
                        ((Activity) context).finish();
                    }
                }
            }
        });
        updateViewState(ControlView.PlayState.Idle);
    }

    private void updateViewState(ControlView.PlayState playState) {

        mControlView.setPlayState(playState);
        if (mTipsView != null) {
            mTipsView.hideBufferLoadingTipView();
        }

        if (playState == ControlView.PlayState.Idle) {
            mGestureView.hide(ViewAction.HideType.Normal);
        } else {
            if (mIsFullScreenLocked) {
                mGestureView.hide(ViewAction.HideType.Normal);
            } else {
                mGestureView.show();
            }

        }
    }

    /**
     * 锁定屏幕。锁定屏幕后，只有锁会显示，其他都不会显示。手势也不可用
     *
     * @param lockScreen 是否锁住
     */
    public void lockScreen(boolean lockScreen) {
        mIsFullScreenLocked = lockScreen;
        mControlView.setScreenLockStatus(mIsFullScreenLocked);
        mGestureView.setScreenLockStatus(mIsFullScreenLocked);
    }

    /**
     * 初始化清晰度列表
     */
    private void initQualityView() {
        mQualityView = new QualityView(getContext());
        addSubView(mQualityView);
        //清晰度点击事件
        mQualityView.setOnQualityClickListener(new QualityView.OnQualityClickListener() {
            @Override
            public void onQualityClick(TrackInfo quality) {
                //进行清晰度的切换
                if (mTipsView != null) {
                    mTipsView.showNetLoadingTipView();
                    mGestureDialogManager.dismissAllDialog();
                }

                //seek中切换清晰度就不在seek中了
                inSeek = false;

                mCicadaVodPlayer.selectTrack(quality.getIndex());

            }
        });
    }

    /**
     * 初始化倍速view
     */
    private void initSpeedView() {
        mSpeedView = new SpeedView(getContext());
        addSubView(mSpeedView);

        //倍速点击事件
        mSpeedView.setOnSpeedClickListener(new SpeedView.OnSpeedClickListener() {
            @Override
            public void onSpeedClick(SpeedView.SpeedValue value) {
                float speed = 1.0f;
                if (value == SpeedView.SpeedValue.Normal) {
                    speed = 1.0f;
                } else if (value == SpeedView.SpeedValue.OneQuartern) {
                    speed = 0.5f;
                } else if (value == SpeedView.SpeedValue.OneHalf) {
                    speed = 1.5f;
                } else if (value == SpeedView.SpeedValue.Twice) {
                    speed = 2.0f;
                }

                //改变倍速
                if (mCicadaVodPlayer != null) {
                    mCicadaVodPlayer.setSpeed(speed);
                }

                mSpeedView.setSpeed(value);
            }

            @Override
            public void onHide() {
                //当倍速界面隐藏之后，显示菜单按钮
//                mControlView.setMenuStatus(true);
            }
        });

    }

    /**
     * 初始化引导view
     */
    private void initGuideView() {
        mGuideView = new GuideView(getContext());
        addSubView(mGuideView);
    }

    /**
     * 切换播放状态。点播播放按钮之后的操作
     */
    private void switchPlayerState() {

        if (currentPlayState == CicadaPlayer.started) {
            pause();
        } else if (currentPlayState == CicadaPlayer.paused || currentPlayState == CicadaPlayer.prepared) {
            start();
        }
    }

    /**
     * 初始化手势view
     */
    private void initGestureView() {
        mGestureView = new GestureView(getContext());
        addSubView(mGestureView);

        //设置手势监听
        mGestureView.setOnGestureListener(new GestureView.GestureListener() {

            @Override
            public void onHorizontalDistance(float downX, float nowX) {
                //水平滑动调节seek。
                // seek需要在手势结束时操作。
                long duration = mCicadaVodPlayer.getDuration();
//                long position = mCicadaVodPlayer.getCurrentPosition();

                long deltaPosition = 0;

                if (currentPlayState == CicadaPlayer.paused ||
                        currentPlayState == CicadaPlayer.prepared ||
                        currentPlayState == CicadaPlayer.started) {
                    //在播放时才能调整大小
                    deltaPosition = (long) (nowX - downX) * duration / getWidth();
                }

//                if (mGestureDialogManager != null) {
//                    mGestureDialogManager.showSeekDialog(AliyunVodPlayerView.this, (int) position);
//                    mGestureDialogManager.updateSeekDialog(duration, position, deltaPosition);
//                }
            }

            @Override
            public void onLeftVerticalDistance(float downY, float nowY) {
                //左侧上下滑动调节亮度
                int changePercent = (int) ((nowY - downY) * 100 / getHeight());

                if (mGestureDialogManager != null) {
                    mGestureDialogManager.showBrightnessDialog(CicadaVodPlayerView.this);
                    int brightness = mGestureDialogManager.updateBrightnessDialog(changePercent);
                    BrightnessUtil.setScreenBrightness((Activity) getContext(), brightness);
                }
            }

            @Override
            public void onRightVerticalDistance(float downY, float nowY) {
                //右侧上下滑动调节音量
                int changePercent = (int) ((nowY - downY) * 100 / getHeight());
                int volume = (int) (mCicadaVodPlayer.getVolume() * 100);

                if (mGestureDialogManager != null) {
                    mGestureDialogManager.showVolumeDialog(CicadaVodPlayerView.this, volume);
                    int targetVolume = mGestureDialogManager.updateVolumeDialog(changePercent);

                    //通过返回值改变音量
                    mCicadaVodPlayer.setVolume(targetVolume / 100.0f);
                }
            }

            @Override
            public void onGestureEnd() {
                //手势结束。
                //seek需要在结束时操作。
                if (mGestureDialogManager != null) {
                    mGestureDialogManager.dismissBrightnessDialog();
                    mGestureDialogManager.dismissVolumeDialog();

                    int seekPosition = mGestureDialogManager.dismissSeekDialog();
                    if (seekPosition >= mCicadaVodPlayer.getDuration()) {
                        seekPosition = (int) (mCicadaVodPlayer.getDuration() - 1000);
                    }

                    if (seekPosition >= 0) {
                        seekTo(seekPosition);
                        inSeek = true;
                    }
                }
            }

            @Override
            public void onSingleTap() {
                //单击事件，显示控制栏
                if (mControlView == null) {
                    return;
                }

//                if (mControlView.getVisibility() != VISIBLE) {
//                    mControlView.show();
//                } else {
//                    mControlView.hide(ControlView.HideType.Normal);
//                }
            }

            @Override
            public void onDoubleTap() {
                //双击事件，控制暂停播放
                switchPlayerState();
            }
        });
    }

    /**
     * 初始化播放器显示view
     */
    private void initSurfaceView() {
        mSurfaceView = new SurfaceView(getContext().getApplicationContext());
        addSubView(mSurfaceView);

        SurfaceHolder holder = mSurfaceView.getHolder();
        //增加surfaceView的监听
        holder.addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                VcPlayerLog.d(TAG, " surfaceCreated = surfaceHolder = " + surfaceHolder);
                if (mCicadaVodPlayer != null) {
                    mCicadaVodPlayer.setDisplay(surfaceHolder);
                    //防止黑屏
                    mCicadaVodPlayer.redraw();
                }
            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int format, int width,
                                       int height) {
                VcPlayerLog.d(TAG, " surfaceChanged surfaceHolder = " + surfaceHolder + " ,  width = " + width + " , height = " + height);
                if (mCicadaVodPlayer != null) {
                    mCicadaVodPlayer.redraw();
                }
            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                VcPlayerLog.d(TAG, " surfaceDestroyed = surfaceHolder = " + surfaceHolder);
                if (mCicadaVodPlayer != null) {
                    mCicadaVodPlayer.setDisplay(null);
                }
            }
        });
    }


    private boolean isPlaying() {
        return currentPlayState == CicadaPlayer.started ||
                currentPlayState == CicadaPlayer.paused;
    }

    private boolean mThumbnailPrepareSuccess = false;

    /**
     * 初始化播放器
     */
    private void initCicadaPlayer() {
        Logger.enableConsoleLog(true);
        mCicadaVodPlayer = CicadaPlayerFactory.createCicadaPlayer(getContext().getApplicationContext());
        mCicadaVodPlayer.enableHardwareDecoder(SharedPreferenceUtils.getBooleanExtra(SharedPreferenceUtils.CICADA_PLAYER_HARDWARE_DECODER));

        //设置准备回调
        mCicadaVodPlayer.setOnPreparedListener(new CicadaPlayer.OnPreparedListener() {

            @Override
            public void onPrepared() {
                if (mCicadaVodPlayer == null) {
                    return;
                }

                generateMediaInfo();
                if (mAliyunMediaInfo != null) {

                    String vodDefinition = "OD";
                    mControlView.setMediaInfo(mAliyunMediaInfo, vodDefinition);

//                    setCoverUri(mAliyunMediaInfo.getCoverUrl());
                }

                mControlView.show();
                if (mTipsView != null) {
                    mTipsView.hideAll();
                }


                if (continuePlay) {
                    if (mControlView != null) {
                        mCicadaVodPlayer.seekTo(mControlView.getVideoPosition(), mSeekMode);
                    }

                    start();
                    continuePlay = false;
                } else {
                    //准备成功之后可以调用start方法开始播放
                    if (mOutPreparedListener != null) {
                        mOutPreparedListener.onPrepared();
                    }
                }

                //重播需要自动播放
                if (isReplay) {
                    start();
                    isReplay = false;
                }
            }
        });

        //播放器出错监听
        mCicadaVodPlayer.setOnErrorListener(new CicadaPlayer.OnErrorListener() {
            @Override
            public void onError(ErrorInfo errorInfo) {
                if (mTipsView != null) {
                    mTipsView.hideAll();
                }
                //出错之后解锁屏幕，防止不能做其他操作，比如返回。
                lockScreen(false);


                showErrorTipView(Integer.toHexString(errorInfo.getCode().getValue()),
                        errorInfo.getCode().getValue() + "", errorInfo.getMsg());

                if (mOutErrorListener != null) {
                    mOutErrorListener.onError(errorInfo);
                }

            }
        });
        //播放器加载回调
        mCicadaVodPlayer.setOnLoadingStatusListener(new CicadaPlayer.OnLoadingStatusListener() {
            @Override
            public void onLoadingBegin() {
                if (mTipsView != null) {
                    mTipsView.hideNetLoadingTipView();
                    mTipsView.showBufferLoadingTipView();
                    mGestureDialogManager.dismissAllDialog();
                }
            }

            @Override
            public void onLoadingEnd() {
                if (mTipsView != null) {
                    mTipsView.hideBufferLoadingTipView();
                }
            }

            @Override
            public void onLoadingProgress(int percent, float sped) {
                if (mTipsView != null) {
                    mTipsView.hideErrorTipView();
                    mTipsView.updateLoadingPercent(percent);
                    if (inSeek) {
                        mTipsView.hideBufferLoadingTipView();
                    }
                }
            }
        });
        //播放结束
        mCicadaVodPlayer.setOnCompletionListener(new CicadaPlayer.OnCompletionListener() {
            @Override
            public void onCompletion() {
                inSeek = false;

                if (mTipsView != null) {
                    //隐藏其他的动作,防止点击界面去进行其他操作
                    mGestureView.hide(ViewAction.HideType.End);
//                    mControlView.hide(ViewAction.HideType.End);
                    mTipsView.showReplayTipView();
                    mGestureDialogManager.dismissAllDialog();
                }

                if (mOutCompletionListener != null) {
                    mOutCompletionListener.onCompletion();
                }
            }
        });

        mCicadaVodPlayer.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
            @Override
            public void onInfo(InfoBean infoBean) {
                if (infoBean.getCode() == InfoCode.BufferedPosition) {
                    mCurrentBufferPercentage = (int) infoBean.getExtraValue();
                    if (mControlView != null) {
                        mControlView.setVideoBufferPosition(mCurrentBufferPercentage);
                    }
                } else if (infoBean.getCode() == InfoCode.AutoPlayStart) {
                    updateViewState(ControlView.PlayState.Playing);
                } else if (infoBean.getCode() == InfoCode.CurrentPosition) {
                    if (mControlView != null) {
                        mControlView.setVideoPosition((int) infoBean.getExtraValue());
                    }
                } else if (infoBean.getCode() == InfoCode.SwitchToSoftwareVideoDecoder) {
                    Toast.makeText(getContext(), getContext().getString(R.string.cicada_change_to_soft_decoder), Toast.LENGTH_SHORT).show();
                } else if (infoBean.getCode() == InfoCode.NetworkRetry) {
                    networkRetry();
                } else if (infoBean.getCode() == InfoCode.NetworkRetrySuccess) {
                    //重连成功后，恢复到默认重连次数
                    mRetryTime = 3;
                    Log.e(TAG, "NetworkRetrySuccess");
                    Toast.makeText(getContext(), R.string.cicada_tip_network_connect_success, Toast.LENGTH_SHORT).show();
                }
                if (mOutInfoListener != null) {
                    mOutInfoListener.onInfo(infoBean);
                }
            }

        });
        //首帧
        mCicadaVodPlayer.setOnRenderingStartListener(new CicadaPlayer.OnRenderingStartListener() {
            @Override
            public void onRenderingStart() {
                inSeek = false;

                boolean changed = generateMediaInfo();
//                if (changed && mAliyunMediaInfo != null) {
//                    TrackInfo trackInfo = mCicadaVodPlayer.currentTrack(TrackInfo.Type.TYPE_VOD.ordinal());
//                    if (trackInfo != null) {
//                        mControlView.setMediaInfo(mAliyunMediaInfo, trackInfo.getVodDefinition());
//                    }
//
//                }
                mCoverView.setVisibility(GONE);
                mTipsView.hideAll();
                updateViewState(ControlView.PlayState.Playing);
            }
        });

        //字幕
        mCicadaVodPlayer.setOnSubtitleDisplayListener(new CicadaPlayer.OnSubtitleDisplayListener() {

            @Override
            public void onSubtitleExtAdded(int trackIndex, String url) {

                VcPlayerLog.e(TAG, "onSubtitleExtAdded : " + " --- trackIndex = " + trackIndex + " --- url = " + url);
                if (mOutSubtitleDisplayListener != null) {
                    mOutSubtitleDisplayListener.onSubtitleExtAdded(trackIndex, url);
                }

            }

            @Override
            public void onSubtitleShow(int trackIndex, long id, String data) {
                SubtitleView.Subtitle subtitle = new SubtitleView.Subtitle();
                subtitle.id = id + "";
                subtitle.content = data;
                subtitleView.show(subtitle);
            }

            @Override
            public void onSubtitleHide(int trackIndex, long id) {
                subtitleView.dismiss(id + "");
            }
        });

        //track变化
        mCicadaVodPlayer.setOnTrackChangedListener(new CicadaPlayer.OnTrackChangedListener() {
            @Override
            public void onChangedSuccess(TrackInfo trackInfo) {
//                if (trackInfo.getType() == TrackInfo.Type.TYPE_VOD) {
//                    mControlView.setCurrentQuality(trackInfo.getVodDefinition());
//                }
                if (mOutTrackChangedListener != null) {
                    mOutTrackChangedListener.onChangedSuccess(trackInfo);
                }
                showToast(trackInfo, true);
                start();

                if (mTipsView != null) {
                    mTipsView.hideNetLoadingTipView();
                }
            }

            @Override
            public void onChangedFail(TrackInfo trackInfo, ErrorInfo errorInfo) {
                if (mTipsView != null) {
                    mTipsView.hideNetLoadingTipView();
                }
                if (mOutTrackChangedListener != null) {
                    mOutTrackChangedListener.onChangedFail(trackInfo, errorInfo);
                }
                showToast(trackInfo, false);
                stop();
            }
        });


        //seek结束事件
        mCicadaVodPlayer.setOnSeekCompleteListener(new CicadaPlayer.OnSeekCompleteListener() {
            @Override
            public void onSeekComplete() {
                inSeek = false;
                long seekTime = System.currentTimeMillis() - startSeekTime;
                if (mOuterSeekCompleteListener != null) {
                    mOuterSeekCompleteListener.onSeekComplete();
                }
            }
        });

        //状态改变监听
        mCicadaVodPlayer.setOnStateChangedListener(new CicadaPlayer.OnStateChangedListener() {
            @Override
            public void onStateChanged(int newState) {
                currentPlayState = newState;
            }
        });


        //截图监听
        mCicadaVodPlayer.setOnSnapShotListener(new CicadaPlayer.OnSnapShotListener() {
            @Override
            public void onSnapShot(Bitmap bitmap, int i, int i1) {
                if (mSnapShotListener != null) {
                    mSnapShotListener.onSnapShot(bitmap, i, i1);
                }
            }
        });

        mCicadaVodPlayer.setScaleMode(CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT);
        mCicadaVodPlayer.setDisplay(mSurfaceView.getHolder());
    }

    private int currentPlayState = CicadaPlayer.idle;

    private boolean generateMediaInfo() {
        boolean changed = false;

        if (mCicadaVodPlayer == null) {
            return false;
        }

        if (mAliyunMediaInfo == null) {
            mAliyunMediaInfo = new MediaInfoExt(mCicadaVodPlayer.getMediaInfo());
            changed = true;
        }

        if (mAliyunMediaInfo == null) {
            changed = false;
        } else {
            //防止服务器信息和实际不一致
            int oldDuration = mAliyunMediaInfo.getDuration();
            int newDuration = (int) mCicadaVodPlayer.getDuration();
            changed = (changed || (oldDuration != newDuration));
            mAliyunMediaInfo.setDuration(newDuration);
        }

        return changed;
    }

    /**
     * 用于提示切换码率,音轨,字幕流成功/失败后的信息提示
     *
     * @param trackInfo 切换TrackInfo
     * @param isSuccess 是否切换成功
     */
    private void showToast(TrackInfo trackInfo, boolean isSuccess) {
        TrackInfo.Type type = trackInfo.getType();
        String result;
        if (type == TrackInfo.Type.TYPE_SUBTITLE) {
            //字幕
            result = isSuccess ? getContext().getString(R.string.cicada_change_subtitle_track) + trackInfo.getSubtitleLang() : getContext().getString(R.string.cicada_change_track_failure);
        } else if (type == TrackInfo.Type.TYPE_AUDIO) {
            //音轨
            result = isSuccess ? getContext().getString(R.string.cicada_change_audio_track) + trackInfo.getAudioLang() : getContext().getString(R.string.cicada_change_track_failure);
        } else if (type == TrackInfo.Type.TYPE_VIDEO) {
            //码率
            result = isSuccess ? getContext().getString(R.string.cicada_change_bitate_track) + trackInfo.getVideoBitrate() : getContext().getString(R.string.cicada_change_track_failure);
        }
//        else if(type == TrackInfo.Type.TYPE_VOD){
//            //清晰度
//            result = isSuccess ? "切换清晰度 " + trackInfo.getVodDefinition() : "切换清晰度失败";
//        }
        else {
            result = getContext().getString(R.string.cicada_change_track_failure);
        }

        Toast.makeText(getContext(), result, Toast.LENGTH_SHORT).show();
    }

    /**
     * 获取从源中设置的标题 。
     * 如果用户设置了标题，优先使用用户设置的标题。
     * 如果没有，就使用服务器返回的标题
     *
     * @param title 服务器返回的标题
     * @return 最后的标题
     */
    private String getTitle(String title) {

        String finalTitle = title;

        if (TextUtils.isEmpty(finalTitle)) {
            return title;
        } else {
            return finalTitle;
        }
    }

    /**
     * 获取从源中设置的封面 。
     * 如果用户设置了封面，优先使用用户设置的封面。
     * 如果没有，就使用服务器返回的封面
     *
     * @param postUrl 服务器返回的封面
     * @return 最后的封面
     */
    private String getPostUrl(String postUrl) {
        String finalPostUrl = postUrl;
        if (TextUtils.isEmpty(finalPostUrl)) {
            return postUrl;
        } else {
            return finalPostUrl;
        }
    }


    /**
     * 获取整体缓冲进度
     *
     * @return 整体缓冲进度
     */
    public int getBufferPercentage() {
        if (mCicadaVodPlayer != null) {
            return mCurrentBufferPercentage;
        }
        return 0;
    }


    /**
     * 获取视频时长
     *
     * @return 视频时长
     */
    public int getDuration() {
        if (mCicadaVodPlayer != null && isPlaying()) {
            return (int) mCicadaVodPlayer.getDuration();
        }

        return 0;
    }


    /**
     * 显示错误提示
     *
     * @param errorCode  错误码
     * @param errorEvent 错误事件
     * @param errorMsg   错误描述
     */
    private void showErrorTipView(String errorCode, String errorEvent, String errorMsg) {

//        pause();
//        stop();

        updateViewState(ControlView.PlayState.Idle);


        if (mTipsView != null) {
            //隐藏其他的动作,防止点击界面去进行其他操作
            mGestureView.hide(ViewAction.HideType.End);
//            mControlView.hide(ViewAction.HideType.End);
//            mCoverView.setVisibility(GONE);
            mTipsView.showErrorTipView(errorCode, errorEvent, errorMsg);

            mGestureDialogManager.dismissAllDialog();
        }
    }

    /**
     * addSubView
     * 添加子view到布局中
     *
     * @param view 子view
     */
    private void addSubView(View view) {
        LayoutParams params = new LayoutParams(LayoutParams.MATCH_PARENT, LayoutParams.MATCH_PARENT);
        //添加到布局中
        addView(view, params);
    }

    /**
     * 添加子View到布局中央
     */
    private void addSubViewByCenter(View view) {
        LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
        params.addRule(RelativeLayout.CENTER_IN_PARENT);
        addView(view, params);
    }

    /**
     * 改变屏幕模式：小屏或者全屏。
     *
     * @param targetMode
     */
    public void changeScreenMode(CicadaScreenMode targetMode, boolean isReverse) {
        VcPlayerLog.d(TAG, "mIsFullScreenLocked = " + mIsFullScreenLocked + " ， targetMode = " + targetMode);

        CicadaScreenMode finalScreenMode = targetMode;

        if (mIsFullScreenLocked) {
            finalScreenMode = CicadaScreenMode.Full;
        }

        //这里可能会对模式做一些修改
        if (targetMode != mCurrentScreenMode) {
            mCurrentScreenMode = finalScreenMode;
        }

        if (mControlView != null) {
            mControlView.setScreenModeStatus(finalScreenMode);
        }

        if (mSpeedView != null) {
            mSpeedView.setScreenMode(finalScreenMode);
        }

        if (mGuideView != null) {
            mGuideView.setScreenMode(finalScreenMode);
        }

        Context context = getContext();
        if (context instanceof Activity) {
            if (finalScreenMode == CicadaScreenMode.Full) {
                if (getLockPortraitMode() == null) {
                    //不是固定竖屏播放。
                    //                    ((Activity) context).setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                    if (isReverse) {
                        ((Activity) context).setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_REVERSE_LANDSCAPE);
                    } else {
                        ((Activity) context).setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
                    }

                    //SCREEN_ORIENTATION_LANDSCAPE只能固定一个横屏方向
                } else {
                    //如果是固定全屏，那么直接设置view的布局，宽高
                    ViewGroup.LayoutParams cicadaVideoViewLayoutParams = getLayoutParams();
                    cicadaVideoViewLayoutParams.height = ViewGroup.LayoutParams.MATCH_PARENT;
                    cicadaVideoViewLayoutParams.width = ViewGroup.LayoutParams.MATCH_PARENT;
                }
            } else if (finalScreenMode == CicadaScreenMode.Small) {

                if (getLockPortraitMode() == null) {
                    //不是固定竖屏播放。
                    ((Activity) context).setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
                } else {
                    //如果是固定全屏，那么直接设置view的布局，宽高
                    ViewGroup.LayoutParams cicadaVideoViewLayoutParams = getLayoutParams();
                    cicadaVideoViewLayoutParams.height = (int) (ScreenUtils.getWidth(context) * 9.0f / 16);
                    cicadaVideoViewLayoutParams.width = ViewGroup.LayoutParams.MATCH_PARENT;
                }
            }
        }
    }

    /**
     * 获取当前屏幕模式：小屏、全屏
     *
     * @return 当前屏幕模式
     */
    public CicadaScreenMode getScregetScreenModeenMode() {
        return mCurrentScreenMode;
    }

    /**
     * 设置准备事件监听
     *
     * @param onPreparedListener 准备事件
     */
    public void setOnPreparedListener(CicadaPlayer.OnPreparedListener onPreparedListener) {
        mOutPreparedListener = onPreparedListener;
    }

    /**
     * 设置错误事件监听
     *
     * @param onErrorListener 错误事件监听
     */
    public void setOnErrorListener(CicadaPlayer.OnErrorListener onErrorListener) {
        mOutErrorListener = onErrorListener;
    }

    /**
     * 设置信息事件监听
     *
     * @param onInfoListener 信息事件监听
     */
    public void setOnInfoListener(CicadaPlayer.OnInfoListener onInfoListener) {
        mOutInfoListener = onInfoListener;
    }

    /**
     * 设置播放完成事件监听
     *
     * @param onCompletionListener 播放完成事件监听
     */
    public void setOnCompletionListener(CicadaPlayer.OnCompletionListener onCompletionListener) {
        mOutCompletionListener = onCompletionListener;
    }

    /**
     * 设置seek结束监听
     *
     * @param onSeekCompleteListener seek结束监听
     */
    public void setOnSeekCompleteListener(CicadaPlayer.OnSeekCompleteListener onSeekCompleteListener) {
        mOuterSeekCompleteListener = onSeekCompleteListener;
    }

    /**
     * 设置码率切换监听
     */
    public void setOnTrackChangedListener(CicadaPlayer.OnTrackChangedListener onTrackChangedListener) {
        this.mOutTrackChangedListener = onTrackChangedListener;
    }

    /**
     * 设置字幕监听
     */
    public void setOnOutSubtitleDisplayListener(CicadaPlayer.OnSubtitleDisplayListener onSubtitleDisplayListener) {
        mOutSubtitleDisplayListener = onSubtitleDisplayListener;
    }


    /**
     * 设置加载状态监听
     *
     * @param onLoadingListener 加载状态监听
     */
    public void setOnLoadingListener(CicadaPlayer.OnLoadingStatusListener onLoadingListener) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setOnLoadingStatusListener(onLoadingListener);
        }
    }

    /**
     * 设置视频宽高变化监听
     *
     * @param onVideoSizeChangedListener 视频宽高变化监听
     */
    public void setOnVideoSizeChangedListener(CicadaPlayer.OnVideoSizeChangedListener onVideoSizeChangedListener) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setOnVideoSizeChangedListener(onVideoSizeChangedListener);
        }
    }

    /**
     * 清空之前设置的播放源
     */
    private void clearAllSource() {
        mUrlSource = null;
    }


    /**
     * 设置边播边存
     *
     * @param enable      是否开启。开启之后会根据maxDuration和maxSize决定有无缓存。
     * @param saveDir     保存目录
     * @param maxDuration 单个文件最大时长 秒
     * @param maxSize     所有文件最大大小 MB
     */
    public void setPlayingCache(boolean enable, String saveDir, int maxDuration, long maxSize) {
        if (mCicadaVodPlayer != null) {
            CacheConfig cacheConfig = new CacheConfig();
            cacheConfig.mEnable = enable;
            cacheConfig.mDir = saveDir;
            cacheConfig.mMaxDurationS = maxDuration;
            cacheConfig.mMaxSizeMB = (int) maxSize;
            mCicadaVodPlayer.setCacheConfig(cacheConfig);
        }
    }

    /**
     * 设置缩放模式
     *
     * @param scallingMode 缩放模式
     */
    public void setVideoScalingMode(CicadaPlayer.ScaleMode scallingMode) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setScaleMode(scallingMode);
        }
    }

    /**
     * 设置截图监听
     */
    public void setSnapShotListener(CicadaPlayer.OnSnapShotListener snapShotListener) {
        this.mSnapShotListener = snapShotListener;
    }

    /**
     * 在activity调用onResume的时候调用。
     * 解决home回来后，画面方向不对的问题
     */
    public void onResume() {
        if (mIsFullScreenLocked) {
            int orientation = getResources().getConfiguration().orientation;
            if (orientation == Configuration.ORIENTATION_PORTRAIT) {
                changeScreenMode(CicadaScreenMode.Small, false);
            } else if (orientation == Configuration.ORIENTATION_LANDSCAPE) {
                changeScreenMode(CicadaScreenMode.Full, false);
            }
        }

        if (mNetWatchdog != null) {
            mNetWatchdog.startWatch();
        }

        if (mOrientationWatchDog != null) {
            mOrientationWatchDog.startWatch();
        }

        //从其他界面过来的话，也要show。
        if (mControlView != null) {
            mControlView.show();
        }

        //onStop中记录下来的状态，在这里恢复使用
        resumePlayerState();
    }

    /**
     * 活动停止时，暂停播放器的操作
     */
    public void onStop() {
        if (mNetWatchdog != null) {
            mNetWatchdog.stopWatch();
        }
        if (mOrientationWatchDog != null) {
            mOrientationWatchDog.stopWatch();
        }

        //保存播放器的状态，供resume恢复使用。
        savePlayerState();
    }

    /**
     * Activity回来后，恢复之前的状态
     */
    private void resumePlayerState() {
        if (mCicadaVodPlayer == null) {
            return;
        }

        start();

    }

    /**
     * 保存当前的状态，供恢复使用
     */
    private void savePlayerState() {
        if (mCicadaVodPlayer == null) {
            return;
        }

        //然后再暂停播放器
        //如果希望后台继续播放，不需要暂停的话，可以注释掉pause调用。
        pause();

    }


    /**
     * 获取媒体信息
     *
     * @return 媒体信息
     */
    public MediaInfo getMediaInfo() {
        if (mCicadaVodPlayer != null) {
            return mCicadaVodPlayer.getMediaInfo();
        }

        return null;
    }

    /**
     * 活动销毁，释放
     */
    public void onDestroy() {
        stop();
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setSurface(null);
            mCicadaVodPlayer.release();
        }
        mSurfaceView = null;

        if (mNetWatchdog != null) {
            mNetWatchdog.stopWatch();
        }
        if (mOrientationWatchDog != null) {
            mOrientationWatchDog.destroy();
        }
    }


    /**
     * 开始播放
     */
    public void start() {

        if (mCicadaVodPlayer == null) {
            return;
        }

        if (currentPlayState == CicadaPlayer.prepared || isPlaying()) {
            mCicadaVodPlayer.start();
            updateViewState(ControlView.PlayState.Playing);
        }

    }

    /**
     * 暂停播放
     */
    public void pause() {
        updateViewState(ControlView.PlayState.Paused);

        if (mCicadaVodPlayer == null) {
            return;
        }

        if (isPlaying()) {
            mCicadaVodPlayer.pause();
        }
    }

    /**
     * 停止播放
     */
    public void stop() {
        if (mCicadaVodPlayer != null) {
     //       mCicadaVodPlayer.selectExtSubtitle(mPreviewTrackIndex, false);
            mCicadaVodPlayer.stop();

            updateViewState(ControlView.PlayState.Idle);
        }
    }

    /**
     * 重新加载
     */
    public void reload() {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.reload();
        }
    }

    /**
     * seek操作
     *
     * @param position 目标位置
     */
    public void seekTo(int position) {
        if (mCicadaVodPlayer == null) {
            return;
        }

        inSeek = true;
        startSeekTime = System.currentTimeMillis();
        mCicadaVodPlayer.seekTo(position, mSeekMode);
    }


    /**
     * 设置是否显示标题栏
     *
     * @param show true:是
     */
    public void setTitleBarCanShow(boolean show) {
        if (mControlView != null) {
            mControlView.setTitleBarCanShow(show);
        }
    }

    /**
     * 设置是否显示控制栏
     *
     * @param show true:是
     */
    public void setControlBarCanShow(boolean show) {
        if (mControlView != null) {
            mControlView.setControlBarCanShow(show);
        }

    }

    /**
     * 开启底层日志
     */
    public void enableNativeLog() {
       Logger.enableConsoleLog(true);
    }


    /**
     * 关闭底层日志
     */
    public void disableNativeLog() {
        Logger.enableConsoleLog(false);
    }

    /**
     * 获取SDK版本号
     *
     * @return SDK版本号
     */
    public String getSDKVersion() {
        return CicadaPlayerFactory.getSdkVersion();
    }

    /**
     * 获取播放surfaceView
     *
     * @return 播放surfaceView
     */
    public SurfaceView getPlayerView() {
        return mSurfaceView;
    }

    /**
     * 设置锁定竖屏监听
     *
     * @param listener 监听器
     */
    public void setLockPortraitMode(LockPortraitListener listener) {
        mLockPortraitListener = listener;
    }

    /**
     * 锁定竖屏
     *
     * @return 竖屏监听器
     */
    public LockPortraitListener getLockPortraitMode() {
        return mLockPortraitListener;
    }

    /**
     * 让home键无效
     *
     * @param keyCode 按键
     * @param event   事件
     * @return 是否处理。
     */
    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        //锁屏，并且不是home键 不处理
        if (mIsFullScreenLocked) {
            return false;
        }

        return true;
    }


    /**
     * 截图功能
     *
     * @return 图片
     */
    public void getSnapShot() {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.snapshot();
        }
    }

    /**
     * 设置循环播放
     *
     * @param circlePlay true:循环播放
     */
    public void setCirclePlay(boolean circlePlay) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setLoop(circlePlay);
        }
    }

    /**
     * 获取AliyunVodPlayer
     */
    public CicadaPlayer getAliyunVodPlayer() {
        return mCicadaVodPlayer;
    }

    /**
     * 准备
     */
    public void prepare() {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.prepare();
        }
    }

    /**
     * 设置缩放模式
     */
    public void setScaleMode(CicadaPlayer.ScaleMode scaleMode) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setScaleMode(scaleMode);
            Toast.makeText(getContext(), getContext().getString(R.string.cicada_scale) + scaleMode, Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 设置镜像模式
     */
    public void setMirrorMode(CicadaPlayer.MirrorMode mirrorMode) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setMirrorMode(mirrorMode);
            Toast.makeText(getContext(), getContext().getString(R.string.cicada_mirror) + mirrorMode, Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 设置旋转模式
     */
    public void setRotationMode(CicadaPlayer.RotateMode rotate) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setRotateMode(rotate);
            Toast.makeText(getContext(), getContext().getString(R.string.cicada_rotate) + rotate, Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 设置倍速播放
     */
    public void setSpeedMode(float speed) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setSpeed(speed);
        }
    }

    /**
     * 设置音量
     */
    public void setVolume(float volume) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setVolume(volume);
        }
    }

    /**
     * 设置播放配置
     *
     * @param referrer                referrer
     * @param maxDelayTime            最大延迟
     * @param httpProxy               http代理
     * @param probeSize               最大probe大小
     * @param netWorkTimeOut          网络超时时间
     * @param hightBufferLevel        高水位时长
     * @param firstStartBufferLevel   起播缓冲区时长
     * @param maxBufferPackedDuration 最大缓冲区时长
     * @param clearFrameWhenStop      停止显示最后帧
     */
    public void setPlayerConfig(String referrer, String maxDelayTime, String httpProxy, String probeSize,
                                String netWorkTimeOut, String hightBufferLevel, String firstStartBufferLevel,
                                String maxBufferPackedDuration, boolean clearFrameWhenStop, String retryCount) {
        if (mCicadaVodPlayer != null) {
            PlayerConfig config = mCicadaVodPlayer.getConfig();

            if (!TextUtils.isEmpty(maxDelayTime)) {
                config.mMaxDelayTime = Integer.valueOf(maxDelayTime);
            }

            if (!TextUtils.isEmpty(probeSize)) {
                config.mMaxProbeSize = Integer.valueOf(probeSize);
            }
            if (!TextUtils.isEmpty(netWorkTimeOut)) {
                config.mNetworkTimeout = Integer.valueOf(netWorkTimeOut);
            }
            if (!TextUtils.isEmpty(hightBufferLevel)) {
                config.mHighBufferDuration = Integer.valueOf(hightBufferLevel);
            }
            if (!TextUtils.isEmpty(firstStartBufferLevel)) {
                config.mStartBufferDuration = Integer.valueOf(firstStartBufferLevel);
            }
            if (!TextUtils.isEmpty(maxBufferPackedDuration)) {
                config.mMaxBufferDuration = Integer.valueOf(maxBufferPackedDuration);
            }
            config.mReferrer = referrer;
            config.mHttpProxy = httpProxy;
            config.mClearFrameWhenStop = clearFrameWhenStop;
            config.mNetworkRetryCount = Integer.valueOf(retryCount);
            mCicadaVodPlayer.setConfig(config);
            Toast.makeText(getContext(), getContext().getString(R.string.cicada_refresh_config_success), Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 设置播放配置
     */
    public void setPlayerConfig(PlayerConfig playerConfig) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setConfig(playerConfig);
            Toast.makeText(getContext(), getContext().getString(R.string.cicada_refresh_config_success), Toast.LENGTH_SHORT).show();
        }
    }


    /**
     * 设置CacheConfig
     *
     * @param maxDuration 最大时长
     * @param maxSize     最大size
     * @param cacheDir    缓存路径
     */
    public void setCacheConfig(String maxDuration, String maxSize, String cacheDir, boolean enableCache) {
        if (mCicadaVodPlayer != null) {
            CacheConfig cacheConfig = new CacheConfig();
            cacheConfig.mEnable = enableCache;
            if (!TextUtils.isEmpty(maxDuration)) {
                cacheConfig.mMaxDurationS = Long.valueOf(maxDuration);
            }
            if (!TextUtils.isEmpty(maxSize)) {
                cacheConfig.mMaxSizeMB = Integer.valueOf(maxSize);
            }
            if (!TextUtils.isEmpty(cacheDir)) {
                cacheConfig.mDir = cacheDir;
            }
            mCicadaVodPlayer.setCacheConfig(cacheConfig);
            Toast.makeText(getContext(), getContext().getString(R.string.cicada_refresh_config_success), Toast.LENGTH_SHORT).show();
        }
    }

    /**
     * 获取配置信息
     */
    public PlayerConfig getPlayerConfig() {
        if (mCicadaVodPlayer != null) {
            return mCicadaVodPlayer.getConfig();
        }
        return null;
    }


    /**
     * 是否循环播放
     */
    public void setLoop(boolean isLoop) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setLoop(isLoop);
        }
    }

    /**
     * 是否静音
     */
    public void setMute(boolean isMute) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setMute(isMute);
        }
    }

    /**
     * 设置自动播放
     *
     * @param auto true 自动播放
     */
    public void setAutoPlay(boolean auto) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.setAutoPlay(auto);
        }
    }

    /**
     * 设置seek模式
     *
     * @param isChecked true为精准seek,false为非精准seek
     */
    public void setSeekMode(boolean isChecked) {
        mSeekMode = isChecked ? CicadaPlayer.SeekMode.Accurate : CicadaPlayer.SeekMode.Inaccurate;
    }

    /**
     * 展示媒体信息
     */
    public void showMediaInfo() {
        if (mCicadaVodPlayer != null) {
            TrackInfo videoBitrateInfo = mCicadaVodPlayer.currentTrack(TrackInfo.Type.TYPE_VIDEO.ordinal());
            StringBuilder stringBuilder = new StringBuilder();

            if (videoBitrateInfo != null) {
                stringBuilder.append(getContext().getString(R.string.cicada_resolution) + ": ")
                        .append(videoBitrateInfo.getVideoHeight())
                        .append("x")
                        .append(videoBitrateInfo.getVideoWidth())
                        .append("\n" + getContext().getString(R.string.cicada_bitrate) + ": ")
                        .append(videoBitrateInfo.getVideoBitrate());
            }
            TrackInfo audioTrackInfo = mCicadaVodPlayer.currentTrack(TrackInfo.Type.TYPE_AUDIO.ordinal());
            if (audioTrackInfo != null) {
                if (stringBuilder.length() > 0) {
                    stringBuilder.append("\n");
                }
                stringBuilder.append(getContext().getString(R.string.cicada_audio) + ": ")
                        .append(audioTrackInfo.getAudioLang());
            }

            TrackInfo subtitleTrackInfo = mCicadaVodPlayer.currentTrack(TrackInfo.Type.TYPE_SUBTITLE.ordinal());
            if (subtitleTrackInfo != null) {
                if (stringBuilder.length() > 0) {
                    stringBuilder.append("\n");
                }
                stringBuilder.append(getContext().getString(R.string.cicada_subtitle) + ": ")
                        .append(subtitleTrackInfo.getSubtitleLang());
            }
            if (stringBuilder.length() > 0) {
                Toast.makeText(getContext(), stringBuilder.toString(), Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(getContext(), getContext().getString(R.string.cicada_none_mediainfo), Toast.LENGTH_SHORT).show();

            }
        }
    }

    public void addSubtitleExt(String url) {
        if (mCicadaVodPlayer != null) {
            VcPlayerLog.e(TAG, "addSubtitleExt = " + url);
            mCicadaVodPlayer.addExtSubtitle(url);
        }
    }


    /**
     * 选择字幕trackIndex
     */
    public void selectSubtitleTrackIndex(int trackIndex) {
        if (currentPlayState == CicadaPlayer.stopped) {
            return;
        }
        if (mCicadaVodPlayer != null) {
            if (trackIndex == -1) {
                mCicadaVodPlayer.selectExtSubtitle(mPreviewTrackIndex, false);
            } else {
                /*
                    切换外挂字幕的时候，先隐藏前一个，再展示选中的
                */
                mCicadaVodPlayer.selectExtSubtitle(mPreviewTrackIndex, false);
                mCicadaVodPlayer.selectExtSubtitle(trackIndex, true);

            }
            mPreviewTrackIndex = trackIndex;
        }
    }

    public void hideSubtitleTrackIndex(int trackIndex) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.selectExtSubtitle(trackIndex, false);
        }
    }

    /**
     * 获取CurrentTrackInfo
     */
    public TrackInfo getCurrentTrackInfo(TrackInfo.Type type) {
        if (mCicadaVodPlayer != null) {
            return mCicadaVodPlayer.currentTrack(type.ordinal());
        }
        return null;
    }

    /**
     * 设置 TrackInfo
     */
    public void selectTrack(TrackInfo trackInfo) {
        if (mCicadaVodPlayer != null) {
            //自动码率
            if (trackInfo == null) {
                mCicadaVodPlayer.selectTrack(TrackInfo.AUTO_SELECT_INDEX);
            } else {
                mCicadaVodPlayer.selectTrack(trackInfo.getIndex());
            }

        }
    }

    /**
     * 获取缩略图
     */
//    public void getThumbnail(CicadaPlayer.OnThumbnailListener thumbnailListener) {
//        if (mCicadaVodPlayer != null) {
//            setOnThumbnailListener(thumbnailListener);
//            mCicadaVodPlayer.getThumbnail(mCicadaVodPlayer.getCurrentPosition());
//    }

    /**
     * 获取音量
     */
    public float getVolume() {
        if (mCicadaVodPlayer != null) {
            return mCicadaVodPlayer.getVolume();
        }
        return 0;
    }

    /**
     * 是否开启硬解
     */
    public void enableHardwareDecoder(boolean enable) {
        if (mCicadaVodPlayer != null) {
            mCicadaVodPlayer.enableHardwareDecoder(enable);
        }
    }
}
