package com.cicada.player.demo.fragment;

import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.InputFilter;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.TextView;

import com.cicada.player.demo.CicadaPlayerActivity;
import com.cicada.player.demo.R;
import com.cicada.player.demo.listener.InputFilterMinMax;
import com.cicada.player.nativeclass.CacheConfig;

/**
 * CacheConfig
 */
public class PlayerCacheConfigFragment extends BaseFragment {

    private View view;
    /**
     * 刷新配置
     */
    private TextView mSaveConfigTextView;
    /**
     * 最大时长
     */
    private EditText mMaxDuration;
    /**
     * 最大size
     */
    private EditText mMaxSize;
    /**
     * 保存路径
     */
    private EditText mCacheDir;
    /**
     * Activity
     */
    private CicadaPlayerActivity mCicadaPlayerActivity;
    /**
     * 停止显示最后帧
     */
    private CheckBox mCacheConfigSwitchCheckBox;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragment_cache_config, container, false);
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

        mMaxDuration = view.findViewById(R.id.et_max_duration);
        mMaxSize = view.findViewById(R.id.et_max_size);
        mCacheDir = view.findViewById(R.id.et_cache_dir);
        mCacheConfigSwitchCheckBox = view.findViewById(R.id.checkbox_cache_config_switch);

        mCacheDir.setHint(Environment.getExternalStorageDirectory().getAbsolutePath() + "/CicadaPlayer_test_cache_save");

    }

    private void initListener() {
        //刷新配置
        mSaveConfigTextView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                saveConfig();
            }
        });

        mMaxDuration.setFilters(new InputFilter[]{new InputFilterMinMax(0,Integer.MAX_VALUE - 1)});
        mMaxSize.setFilters(new InputFilter[]{new InputFilterMinMax(0, Integer.MAX_VALUE - 1)});
    }

    /**
     * 刷新配置
     */
    private void saveConfig() {
        String maxDuration = mMaxDuration.getText().toString();
        String maxSize = mMaxSize.getText().toString();
        String cacheDir = mCacheDir.getText().toString();
        if(TextUtils.isEmpty(cacheDir)){
            cacheDir = mCacheDir.getHint().toString();
            mCacheDir.setText(cacheDir);
        }
        boolean enableCache = mCacheConfigSwitchCheckBox.isChecked();


        mCicadaPlayerActivity.setCacheConfig(maxDuration, maxSize, cacheDir,enableCache);
    }

    /**
     * 获取配置
     */
    private void getConfig(){
        CacheConfig cacheConfig = new CacheConfig();
        mMaxDuration.setText(300+"");
        mMaxSize.setText(200+"");
        mCacheDir.setText(cacheConfig.mDir);
    }

}
