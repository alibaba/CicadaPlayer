package com.cicada.player.demo.fragment;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.InputFilter;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import com.cicada.player.demo.CicadaPlayerActivity;
import com.cicada.player.demo.R;
import com.cicada.player.demo.listener.InputFilterMinMax;
import com.cicada.player.nativeclass.PlayerConfig;

/**
 * playerConfig
 */
public class PlayerConfigFragment extends BaseFragment {

    private View view;
    /**
     * 刷新配置
     */
    private TextView mSaveConfigTextView;
    /**
     * referrer
     */
    private EditText mReferrerEditText;
    /**
     * 最大延迟
     */
    private EditText mMaxDelayTime;
    /**
     * http代理
     */
    private EditText mHttpProxyEditText;
    /**
     * 最大probe大小
     */
    private EditText mProbeSizeEditText;
    /**
     * 网络超时时间
     */
    private EditText mNetWorkTimeOutEditText;
    /**
     * 高水位时长
     */
    private EditText mHightBufferLevelEditText;
    /**
     * 起播缓冲区时长
     */
    private EditText mStartBufferDurationEditText;
    /**
     * 最大缓冲区时长
     */
    private EditText mMaxBufferPacketDurationEditText;
    /**
     * 重试次数
     */
    private EditText mRetryCountEditText;

    /**
     * Activity
     */
    private CicadaPlayerActivity mCicadaPlayerActivity;
    /**
     * 停止显示最后帧
     */
    private CheckBox mClearFrameWhenStopCheckBox;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragment_player_config, container, false);
        initView();
        return view;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mCicadaPlayerActivity = (CicadaPlayerActivity) getActivity();
        initListener();
        getConfig();
    }

    private void initView() {
        mSaveConfigTextView = view.findViewById(R.id.tv_save_config);

        mMaxDelayTime = view.findViewById(R.id.et_max_delay_time);
        mReferrerEditText = view.findViewById(R.id.et_referrer);
        mHttpProxyEditText = view.findViewById(R.id.et_http_proxy);
        mProbeSizeEditText = view.findViewById(R.id.et_probe_size);
        mRetryCountEditText = view.findViewById(R.id.et_retry_count);
        mNetWorkTimeOutEditText = view.findViewById(R.id.et_net_work_time_out);
        mHightBufferLevelEditText = view.findViewById(R.id.et_high_buffer_level);
        mStartBufferDurationEditText = view.findViewById(R.id.et_first_start_buffer_level);
        mClearFrameWhenStopCheckBox = view.findViewById(R.id.checkbox_clear_frame_when_stop);
        mMaxBufferPacketDurationEditText = view.findViewById(R.id.et_max_buffer_packet_duration);

    }

    private void initListener() {
        //刷新配置
        mSaveConfigTextView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                saveConfig();
            }
        });

        mMaxDelayTime.setFilters(new InputFilter[]{new InputFilterMinMax(0,Integer.MAX_VALUE - 1)});
        mProbeSizeEditText.setFilters(new InputFilter[]{new InputFilterMinMax(0, Integer.MAX_VALUE - 1)});
        mNetWorkTimeOutEditText.setFilters(new InputFilter[]{new InputFilterMinMax(0, Integer.MAX_VALUE - 1)});
        mHightBufferLevelEditText.setFilters(new InputFilter[]{new InputFilterMinMax(0, Integer.MAX_VALUE - 1)});
        mStartBufferDurationEditText.setFilters(new InputFilter[]{new InputFilterMinMax(0, Integer.MAX_VALUE - 1)});
        mMaxBufferPacketDurationEditText.setFilters(new InputFilter[]{new InputFilterMinMax(0, Integer.MAX_VALUE - 1)});
    }

    /**
     * 刷新配置
     */
    private void saveConfig() {
        String referrer = mReferrerEditText.getText().toString();
        String maxDelayTime = mMaxDelayTime.getText().toString();
        String httpProxy = mHttpProxyEditText.getText().toString();
        String probeSize = mProbeSizeEditText.getText().toString();
        String netWorkTimeOut = mNetWorkTimeOutEditText.getText().toString();
        boolean clearFrameWhenStop = mClearFrameWhenStopCheckBox.isChecked();
        String hightBufferLevel = mHightBufferLevelEditText.getText().toString();
        String firstStartBufferLevel = mStartBufferDurationEditText.getText().toString();
        String maxBufferPackedDuration = mMaxBufferPacketDurationEditText.getText().toString();
        String retryCount = mRetryCountEditText.getText().toString();

        mCicadaPlayerActivity.setPlayerConfig(referrer, maxDelayTime, httpProxy, probeSize, netWorkTimeOut, hightBufferLevel,
                firstStartBufferLevel, maxBufferPackedDuration,clearFrameWhenStop,retryCount);
    }

    /**
     * 获取配置
     */
    private void getConfig(){
        PlayerConfig playConfig = mCicadaPlayerActivity.getPlayConfig();
        if(mPlaySource.startsWith("artp")){
            mMaxDelayTime.setText("100");
        }else{
            mMaxDelayTime.setText(playConfig.mMaxDelayTime+"");
        }

        mReferrerEditText.setText(playConfig.mReferrer+"");
        mHttpProxyEditText.setText(playConfig.mHttpProxy);
        mProbeSizeEditText.setText(playConfig.mMaxProbeSize+"");
        mNetWorkTimeOutEditText.setText(playConfig.mNetworkTimeout+"");
        mHightBufferLevelEditText.setText(playConfig.mHighBufferDuration+"");
        mClearFrameWhenStopCheckBox.setChecked(playConfig.mClearFrameWhenStop);
        mStartBufferDurationEditText.setText(playConfig.mStartBufferDuration+"");
        mMaxBufferPacketDurationEditText.setText(playConfig.mMaxBufferDuration+"");
        mRetryCountEditText.setText(2+"");

        //初始化后刷新一次配置
        saveConfig();
    }

}
