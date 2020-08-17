package com.cicada.player.demo;

import android.os.Build;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.cicada.player.CicadaPlayerFactory;
import com.cicada.player.demo.util.Common;
import com.cicada.player.demo.util.SharedPreferenceUtils;

/**
 * 设置
 */
public class SettingActivity extends BaseActivity {

    /**
     * 版本号
     */
    private TextView mVersionTextView;
    /**
     * 是否开启硬解
     */
    private CheckBox mHardwareDecoderCheckBox;
    /**
     * 添加黑名单
     */
    private Button mBlackListButton;
    /**
     * phone Model info
     */
    private TextView mModelTextView;
    private RadioGroup mPlayerSelectRadioGroup;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_setting);

        initView();
        initData();
        initListener();
    }

    private void initView(){
        TextView tvTitle = findViewById(R.id.tv_title);
        tvTitle.setText(R.string.title_setting);

        mModelTextView = findViewById(R.id.tv_model);
        mBlackListButton = findViewById(R.id.blacklit);
        mVersionTextView = findViewById(R.id.tv_version);
        mHardwareDecoderCheckBox = findViewById(R.id.hardwaredecoder);
        mPlayerSelectRadioGroup = findViewById(R.id.radio_group_player);
    }

    private void initData(){
        mVersionTextView.setText(CicadaPlayerFactory.getSdkVersion());
        mHardwareDecoderCheckBox.setChecked(SharedPreferenceUtils.getBooleanExtra(SharedPreferenceUtils.CICADA_PLAYER_HARDWARE_DECODER));
        boolean selectedCicadaPlayer = SharedPreferenceUtils.getBooleanExtra(SharedPreferenceUtils.SELECTED_CICADA_PLAYER);
        mPlayerSelectRadioGroup.check(selectedCicadaPlayer ? R.id.radio_btn_cicada : R.id.radio_btn_exo);
        mModelTextView.setText(Build.MODEL);
    }

    private void initListener(){
        mHardwareDecoderCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(Common.HAS_ADD_BLACKLIST && b){
                    Toast.makeText(SettingActivity.this, getString(R.string.cicada_unable_start_hardware_decoder), Toast.LENGTH_SHORT).show();
                    mHardwareDecoderCheckBox.setChecked(false);
                }else{
                    SharedPreferenceUtils.putBooleanExtra(SharedPreferenceUtils.CICADA_PLAYER_HARDWARE_DECODER,b);
                }

            }
        });

        mBlackListButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if(mHardwareDecoderCheckBox != null){
                    mHardwareDecoderCheckBox.setChecked(false);
                }
                Toast.makeText(SettingActivity.this, getString(R.string.cicada_success), Toast.LENGTH_SHORT).show();
                Common.HAS_ADD_BLACKLIST = true;
                CicadaPlayerFactory.DeviceInfo deviceInfo = new CicadaPlayerFactory.DeviceInfo();
                deviceInfo.model = Build.MODEL;
                CicadaPlayerFactory.addBlackDevice(CicadaPlayerFactory.BlackType.HW_Decode_H264,deviceInfo);
            }
        });

        mPlayerSelectRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup radioGroup, int checkedId) {
                SharedPreferenceUtils.putBooleanExtra(SharedPreferenceUtils.SELECTED_CICADA_PLAYER,checkedId == R.id.radio_btn_cicada);
            }
        });
    }
}
