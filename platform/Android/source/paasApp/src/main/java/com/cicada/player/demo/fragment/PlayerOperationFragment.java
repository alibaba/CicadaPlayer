package com.cicada.player.demo.fragment;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.cicada.player.CicadaPlayer;
import com.cicada.player.bean.InfoBean;
import com.cicada.player.bean.InfoCode;
import com.cicada.player.demo.CicadaPlayerActivity;
import com.cicada.player.demo.R;
import com.cicada.player.demo.util.SharedPreferenceUtils;
import com.cicada.player.demo.util.ThreadUtils;


/**
 * 播放视频操作Fragment
 */
public class PlayerOperationFragment extends BaseFragment {

    private View view;
    /**
     * 音量
     */
    private SeekBar mVolumeProgress;
    /**
     * 倍速
     */
    private RadioGroup mSpeedModeRadioGoup;
    /**
     * 缩放模式
     */
    private RadioGroup mScaleModeRadioGroup;
    /**
     * 镜像模式
     */
    private RadioGroup mMirrorModeRadioGroup;
    /**
     * 旋转模式
     */
    private RadioGroup mRotationModeRadioGroup;
    /**
     * Activity
     */
    private CicadaPlayerActivity mCicadaPlayerActivity;
    /**
     * 循环
     */
    private CheckBox mLoopCheckBox;
    /**
     * 静音
     */
    private CheckBox mMuteCheckBox;
    /**
     * 自动播放
     */
    private CheckBox mAutoPlayCheckBox;
    /**
     * 硬解码
     */
    private CheckBox mHardwareDecoderCheckBox;
    /**
     * 媒体信息
     */
    private TextView showMediaInfoTextView;
    /**
     * 黑名单
     */
    private CheckBox mBlackListCheckBox;
    /**
     * 是否开启精准seek
     */
    private CheckBox mAccurateSeekCheckBox;
    /**
     * 是否允许后台播放
     */
    private CheckBox mEnablePlaybackCheckBox;


    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragment_player_operation, container, false);
        initView();
        return view;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mCicadaPlayerActivity = (CicadaPlayerActivity) getActivity();
        initVolume();
        initSpeedMode();
        initScaleMode();
        initMirrorMode();
        initRotationMode();

        initLoop();
        initMute();
        initAutoPlay();
        initHardwareDecoder();
        initShowMedaiInfo();
        initBlackList();
        initAccurateSeek();
        initPlayBack();
    }

    private void initView() {
        mMuteCheckBox = view.findViewById(R.id.muteOn);
        mLoopCheckBox = view.findViewById(R.id.loopOn);
        mAutoPlayCheckBox = view.findViewById(R.id.autoPlayON);
        mBlackListCheckBox = view.findViewById(R.id.blacklist);
        mAccurateSeekCheckBox = view.findViewById(R.id.accurate_seek);
        mHardwareDecoderCheckBox = view.findViewById(R.id.hardwaredecoder);

        mVolumeProgress = view.findViewById(R.id.volumeProgress);

        mSpeedModeRadioGoup = view.findViewById(R.id.speedMode);
        mScaleModeRadioGroup = view.findViewById(R.id.scalingMode);
        mMirrorModeRadioGroup = view.findViewById(R.id.mirrorMode);
        mRotationModeRadioGroup = view.findViewById(R.id.rotationMode);

        showMediaInfoTextView = view.findViewById(R.id.tv_show_mediainfo);
        mEnablePlaybackCheckBox = view.findViewById(R.id.enable_playback);
    }

    /**
     * 倍速播放
     */
    private void initSpeedMode() {
        mSpeedModeRadioGoup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId) {
                    case R.id.speed10:
                        //正常倍速
                        mCicadaPlayerActivity.setSpeedMode(1.0f);
                        break;
                    case R.id.speed05:
                        //0.5倍速
                        mCicadaPlayerActivity.setSpeedMode(0.5f);
                        break;
                    case R.id.speed15:
                        //1.5倍速
                        mCicadaPlayerActivity.setSpeedMode(1.5f);
                        break;
                    case R.id.speed20:
                        //2倍速
                        mCicadaPlayerActivity.setSpeedMode(2.0f);
                        break;
                    default:
                        mCicadaPlayerActivity.setSpeedMode(1.0f);
                        break;
                }
            }
        });
        ((RadioButton) mSpeedModeRadioGoup.findViewById(R.id.speed10)).setChecked(true);
    }

    /**
     * 缩放模式
     */
    private void initScaleMode() {
        ((RadioButton) mScaleModeRadioGroup.findViewById(R.id.fit)).setChecked(true);
        mScaleModeRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId) {
                    case R.id.fit:
                        //比例填充
                        mCicadaPlayerActivity.setScaleMode(CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT);
                        break;
                    case R.id.fill:
                        //比例全屏
                        mCicadaPlayerActivity.setScaleMode(CicadaPlayer.ScaleMode.SCALE_ASPECT_FILL);
                        break;
                    case R.id.fill_only:
                        //拉伸全屏
                        mCicadaPlayerActivity.setScaleMode(CicadaPlayer.ScaleMode.SCALE_TO_FILL);
                        break;
                    default:
                        mCicadaPlayerActivity.setScaleMode(CicadaPlayer.ScaleMode.SCALE_ASPECT_FIT);
                        break;
                }
            }
        });
    }

    /**
     * 镜像模式
     */
    private void initMirrorMode() {
        ((RadioButton) mMirrorModeRadioGroup.findViewById(R.id.mirror_none)).setChecked(true);
        mMirrorModeRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId) {
                    case R.id.mirror_none:
                        //无镜像
                        mCicadaPlayerActivity.setMirrorMode(CicadaPlayer.MirrorMode.MIRROR_MODE_NONE);
                        break;
                    case R.id.mirror_h:
                        //水平镜像
                        mCicadaPlayerActivity.setMirrorMode(CicadaPlayer.MirrorMode.MIRROR_MODE_HORIZONTAL);
                        break;
                    case R.id.mirror_v:
                        //垂直镜像
                        mCicadaPlayerActivity.setMirrorMode(CicadaPlayer.MirrorMode.MIRROR_MODE_VERTICAL);
                        break;
                    default:
                        mCicadaPlayerActivity.setMirrorMode(CicadaPlayer.MirrorMode.MIRROR_MODE_NONE);
                        break;
                }
            }
        });
    }

    /**
     * 旋转模式
     */
    private void initRotationMode() {
        ((RadioButton) mRotationModeRadioGroup.findViewById(R.id.rotate0)).setChecked(true);
        mRotationModeRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                switch (checkedId) {
                    case R.id.rotate0:
                        mCicadaPlayerActivity.setRotationMode(CicadaPlayer.RotateMode.ROTATE_0);
                        break;
                    case R.id.rotate90:
                        mCicadaPlayerActivity.setRotationMode(CicadaPlayer.RotateMode.ROTATE_90);
                        break;
                    case R.id.rotate180:
                        mCicadaPlayerActivity.setRotationMode(CicadaPlayer.RotateMode.ROTATE_180);
                        break;
                    case R.id.rotate270:
                        mCicadaPlayerActivity.setRotationMode(CicadaPlayer.RotateMode.ROTATE_270);
                        break;
                    default:
                        mCicadaPlayerActivity.setRotationMode(CicadaPlayer.RotateMode.ROTATE_0);
                        break;
                }
            }
        });
    }

    /**
     * 音量
     */
    private void initVolume() {
        mVolumeProgress.setMax(100);
        mVolumeProgress.setProgress((int) (getCurrentVolume() * 50));
        mVolumeProgress.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                float volume = seekBar.getProgress() / 50f;
                if (mMuteCheckBox != null && mMuteCheckBox.isChecked()) {
                    mMuteCheckBox.setChecked(false);
                }
                mCicadaPlayerActivity.setVolume(volume);
            }
        });
    }

    /**
     * 循环
     */
    private void initLoop() {
        if (mCicadaPlayerActivity != null) {
            mCicadaPlayerActivity.setLoop(mLoopCheckBox.isChecked());
        }
        mLoopCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (mCicadaPlayerActivity != null) {
                    mCicadaPlayerActivity.setLoop(isChecked);
                }
            }
        });
    }

    /**
     * 静音
     */
    private void initMute() {
        if (mCicadaPlayerActivity != null) {
            mCicadaPlayerActivity.setMute(mMuteCheckBox.isChecked());
        }
        mMuteCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (mCicadaPlayerActivity != null) {
                    mCicadaPlayerActivity.setMute(isChecked);
                }
            }
        });
    }

    /**
     * 自动播放
     */
    private void initAutoPlay() {
        if (mCicadaPlayerActivity != null) {
            mCicadaPlayerActivity.setAutoPlay(mAutoPlayCheckBox.isChecked());
        }
        mAutoPlayCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (mCicadaPlayerActivity != null) {
                    mCicadaPlayerActivity.setAutoPlay(isChecked);
                }
            }
        });
    }

    /**
     * 展示媒体信息
     */
    private void initShowMedaiInfo() {
        showMediaInfoTextView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mCicadaPlayerActivity != null) {
                    mCicadaPlayerActivity.showMediaInfo();
                }
            }
        });
    }

    /**
     * 硬解码
     */
    private void initHardwareDecoder() {
        mHardwareDecoderCheckBox.setChecked(SharedPreferenceUtils.getBooleanExtra(SharedPreferenceUtils.CICADA_PLAYER_HARDWARE_DECODER));

        if (mCicadaPlayerActivity != null) {
            mCicadaPlayerActivity.setOnInfoListener(new CicadaPlayer.OnInfoListener() {
                @Override
                public void onInfo(InfoBean infoBean) {
                    if (infoBean.getCode() == InfoCode.SwitchToSoftwareVideoDecoder) {
                        ThreadUtils.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(getContext(), "切换到软解", Toast.LENGTH_SHORT).show();
                                if (mHardwareDecoderCheckBox != null) {
                                    mHardwareDecoderCheckBox.setChecked(false);
                                }
                            }
                        });
                    } else if (infoBean.getCode() == InfoCode.CacheError) {
                        ThreadUtils.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(getContext(), "缓存失败：" + infoBean.getExtraMsg(), Toast.LENGTH_SHORT).show();
                            }
                        });
                    } else if (infoBean.getCode() == InfoCode.CacheSuccess) {
                        ThreadUtils.runOnUiThread(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(getContext(), "缓存成功", Toast.LENGTH_SHORT).show();
                            }
                        });
                    }
                }
            });
        }
    }

    /**
     * 黑名单
     */
    private void initBlackList() {
        mBlackListCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {

            }
        });
    }

    private void initAccurateSeek(){
        mAccurateSeekCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
                if (mCicadaPlayerActivity != null) {
                    mCicadaPlayerActivity.setSeekMode(isChecked);
                }
            }
        });
    }

    private void initPlayBack()
    {
        mEnablePlaybackCheckBox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked)
            {
                if (mCicadaPlayerActivity != null) {
                    mCicadaPlayerActivity.enablePlayBack(isChecked);
                }
            }
        });
    }

    /**
     * 获取当前音量
     */
    private float getCurrentVolume() {
        if (mCicadaPlayerActivity != null) {
            return mCicadaPlayerActivity.getcurrentVolume();
        }
        return 0;
    }
}
