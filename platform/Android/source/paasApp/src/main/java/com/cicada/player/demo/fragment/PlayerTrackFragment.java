package com.cicada.player.demo.fragment;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.TextUtils;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.cicada.player.demo.CicadaPlayerActivity;
import com.cicada.player.demo.R;
import com.cicada.player.nativeclass.MediaInfo;
import com.cicada.player.nativeclass.TrackInfo;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Locale;
import java.util.Set;

/**
 * track信息Fragment
 */
public class PlayerTrackFragment extends BaseFragment {

    private View view;
    /**
     * Activity
     */
    private CicadaPlayerActivity mApsraPlayerActivity;
    /**
     * 码率
     */
    private RadioGroup mBitrateRadioGroup;
    private LinearLayout mBitrateLinearLayout;
    /**
     * 字幕
     */
    private RadioGroup mSubtitleRadioGroup;
    private LinearLayout mSubtitleLinearLayout;
    /**
     * 清晰度
     */
    private RadioGroup mDefinitionRadioGroup;
    private LinearLayout mDefinitionLinearLayout;
    /**
     * 音轨
     */
    private RadioGroup mSoundTrackRadioGroup;
    private LinearLayout mSoundTrackLinearLayout;
    /**
     * 外挂字幕
     */
    private RadioGroup mSubtitleExtRadioGroup;
    private TrackInfo trackInfo;
    private RadioButton mAutovideoBtn;

    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        view = inflater.inflate(R.layout.fragment_player_track, container, false);
        initView();
        return view;
    }

    @Override
    public void onActivityCreated(@Nullable Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        mApsraPlayerActivity = (CicadaPlayerActivity) getActivity();
        initAll();
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        if(!hidden){
            initAll();
        }
    }

    /**
     * 初始化
     */
    public void initAll(){
        showMediaInfo();
        updateCurrentTrackInfo();
        initBitrateListener();
        initSubtitleListener();
        initDefinitionListener();
        initSoundTrackListener();
        initSubtitleExtListener();
    }

    private void initView() {
        mBitrateRadioGroup = view.findViewById(R.id.bitrate);
        mSubtitleRadioGroup = view.findViewById(R.id.subtitle);
        mDefinitionRadioGroup = view.findViewById(R.id.definition);
        mSoundTrackRadioGroup = view.findViewById(R.id.soundtrack);
        mSubtitleExtRadioGroup = view.findViewById(R.id.subtitle_ext);

        mBitrateLinearLayout = view.findViewById(R.id.ll_bitrate);
        mSubtitleLinearLayout = view.findViewById(R.id.ll_subtitle);
        mDefinitionLinearLayout = view.findViewById(R.id.ll_definition);
        mSoundTrackLinearLayout = view.findViewById(R.id.ll_soundtrack);

        mBitrateRadioGroup.setOrientation(LinearLayout.VERTICAL);
        mSubtitleRadioGroup.setOrientation(LinearLayout.VERTICAL);
        mDefinitionRadioGroup.setOrientation(LinearLayout.VERTICAL);
        mSoundTrackRadioGroup.setOrientation(LinearLayout.VERTICAL);
    }

    private void showMediaInfo() {
        if (mApsraPlayerActivity != null) {
            MediaInfo mediaInfo = mApsraPlayerActivity.getMediaInfo();
            LinkedHashMap<Integer, String> mSubtitleMap = mApsraPlayerActivity.getSubtitleMap();
            //码率
            mBitrateRadioGroup.removeAllViews();
            //字幕信息
            mSubtitleRadioGroup.removeAllViews();
            //音轨信息
            mSoundTrackRadioGroup.removeAllViews();
            //清晰度信息
            mDefinitionRadioGroup.removeAllViews();
            //外挂字幕
            mSubtitleExtRadioGroup.removeAllViews();

            if (mediaInfo == null) {
                Toast.makeText(mApsraPlayerActivity.getApplicationContext(), getString(R.string.cicada_get_mediainfo_failure), Toast.LENGTH_SHORT).show();
                return;
            }
            List<TrackInfo> trackInfos = mediaInfo.getTrackInfos();
            for (TrackInfo trackInfo : trackInfos) {
                TrackInfo.Type type = trackInfo.getType();
                if (type == TrackInfo.Type.TYPE_SUBTITLE) {
                    if(!TextUtils.isEmpty(trackInfo.getSubtitleLang())){
                        RadioButton subtitleBtn = createRadioButton();
                        subtitleBtn.setTag(trackInfo);
                        subtitleBtn.setText(trackInfo.getSubtitleLang());
                        mSubtitleRadioGroup.addView(subtitleBtn);
                    }
                } else if (type == TrackInfo.Type.TYPE_AUDIO) {
                    if(!TextUtils.isEmpty(trackInfo.getAudioLang())){
                        RadioButton audioBtn = createRadioButton();
                        audioBtn.setTag(trackInfo);
                        audioBtn.setText(trackInfo.getAudioLang());
                        mSoundTrackRadioGroup.addView(audioBtn);
                    }
                } else if (type == TrackInfo.Type.TYPE_VIDEO) {
                    if(mBitrateRadioGroup.getChildCount() == 0){
                        mAutovideoBtn = createRadioButton();
                        mAutovideoBtn.setText(getString(R.string.auto_bitrate));
                        mBitrateRadioGroup.addView(mAutovideoBtn);
                    }
                    if(trackInfo.getVideoBitrate() > 0){
                        RadioButton videoBtn = createRadioButton();
                        videoBtn.setTag(trackInfo);
                        videoBtn.setText(trackInfo.getVideoBitrate() + "");
                        mBitrateRadioGroup.addView(videoBtn);
                    }
                }
//                else if (type == TrackInfo.Type.TYPE_VOD) {
//                    if(!TextUtils.isEmpty(trackInfo.getVodDefinition())){
//                        RadioButton mixBtn = createRadioButton();
//                        mixBtn.setTag(trackInfo);
//                        mixBtn.setText(trackInfo.getVodDefinition());
//                        mDefinitionRadioGroup.addView(mixBtn);
//                    }
//                }
            }

            //添加外挂字幕
            if (mSubtitleMap != null && mSubtitleMap.size() != 0) {
                mSubtitleMap.put(-1,getString(R.string.cicada_cancel_subtitle_ext));
                Set<Integer> keyTrackIndex = mSubtitleMap.keySet();
                for (Integer trackIndex : keyTrackIndex) {
                    RadioButton subtitleExtCnBtn = createRadioButton();
                    subtitleExtCnBtn.setTag(trackIndex);
                    subtitleExtCnBtn.setText(mSubtitleMap.get(trackIndex));
                    mSubtitleExtRadioGroup.addView(subtitleExtCnBtn);
                }
            }

            hideRadioGroup();
        }
    }

    /**
     * 创建RadioButton
     */
    private RadioButton createRadioButton(){
        final RadioButton radioButton = new RadioButton(mApsraPlayerActivity);
        radioButton.post(new Runnable() {
            @Override
            public void run() {
                LinearLayout.LayoutParams layoutParams = (LinearLayout.LayoutParams) radioButton.getLayoutParams();
                layoutParams.width = LinearLayout.LayoutParams.MATCH_PARENT;
                layoutParams.height = LinearLayout.LayoutParams.WRAP_CONTENT;
                radioButton.setLayoutParams(layoutParams);
            }
        });

        radioButton.setGravity(Gravity.CENTER);
        radioButton.setButtonDrawable(null);
        radioButton.setBackground(getResources().getDrawable(R.drawable.radio_rect_bkg));
        return radioButton;
    }

    /**
     * 隐藏没有选项的RadioGroup
     */
    private void hideRadioGroup() {
        if(mBitrateRadioGroup.getChildCount() <= 0){
            mBitrateLinearLayout.setVisibility(View.GONE);
        }
        if(mSoundTrackRadioGroup.getChildCount() <= 0){
            mSoundTrackLinearLayout.setVisibility(View.GONE);
        }
        if(mDefinitionRadioGroup.getChildCount() <= 0){
            mDefinitionLinearLayout.setVisibility(View.GONE);
        }
        if(mSubtitleRadioGroup.getChildCount() <= 0){
            mSubtitleLinearLayout.setVisibility(View.GONE);
        }
    }

    /**
     * 获取不同的流信息
     */
    private void updateCurrentTrackInfo() {
//        updateTrackInfoViewByType(TrackInfo.Type.TYPE_VOD, mDefinitionRadioGroup);
        updateTrackInfoViewByType(TrackInfo.Type.TYPE_VIDEO, mBitrateRadioGroup);
        updateTrackInfoViewByType(TrackInfo.Type.TYPE_AUDIO, mSoundTrackRadioGroup);
        updateTrackInfoViewByType(TrackInfo.Type.TYPE_SUBTITLE, mSubtitleRadioGroup);
    }

    private void updateTrackInfoViewByType(TrackInfo.Type type, RadioGroup group) {
        if (mApsraPlayerActivity != null) {
            trackInfo = mApsraPlayerActivity.getCurrentTrackInfo(type);
        }
        if (trackInfo != null) {
            int childSize = group.getChildCount();
            for (int i = 0; i < childSize; i++) {
                RadioButton childAt = (RadioButton) group.getChildAt(i);
                TrackInfo tag = (TrackInfo) childAt.getTag();
                //自动码率
                if(tag == null && type == TrackInfo.Type.TYPE_VIDEO){
                    childAt.setChecked(true);
                    childAt.setText(getString(R.string.auto_bitrate)+"(" + trackInfo.getVideoBitrate() +")");
                    continue;
                }
                if (tag.getIndex() == trackInfo.getIndex()) {
                    childAt.setChecked(true);
                    break;
                }
            }
        }
    }

    /**
     * 设置码率切换监听
     */
    private void initBitrateListener() {
        mBitrateRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(mApsraPlayerActivity != null){
                    RadioButton checkedItem = group.findViewById(checkedId);
                    TrackInfo trackInfo = (TrackInfo) checkedItem.getTag();
                    mApsraPlayerActivity.selectTrack(trackInfo);
                }
            }
        });
    }

    /**
     * 设置字幕切换监听
     */
    private void initSubtitleListener() {
        mSubtitleRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                if(mApsraPlayerActivity != null){
                    RadioButton checkItem = group.findViewById(checkedId);
                    TrackInfo trackInfo = (TrackInfo) checkItem.getTag();
                    mApsraPlayerActivity.selectTrack(trackInfo);
                }
            }
        });
    }

    /**
     * 设置清晰度切换监听
     */
    private void initDefinitionListener() {
        mDefinitionRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                RadioButton checkItem = group.findViewById(checkedId);
                TrackInfo trackInfo = (TrackInfo) checkItem.getTag();
                mApsraPlayerActivity.selectTrack(trackInfo);
            }
        });
    }

    /**
     * 设置音轨切换监听
     */
    private void initSoundTrackListener() {
        mSoundTrackRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                RadioButton checkItem = group.findViewById(checkedId);
                TrackInfo trackInfo = (TrackInfo) checkItem.getTag();
                mApsraPlayerActivity.selectTrack(trackInfo);
            }
        });
    }

    /**
     * 外挂字幕切换监听
     */
    private void initSubtitleExtListener(){
        mSubtitleExtRadioGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                RadioButton radioButton = group.findViewById(checkedId);
                int trackIndex = (int) radioButton.getTag();
                mApsraPlayerActivity.selectSubtitleTrackIndex(trackIndex);
            }
        });
    }

    /**
     * track改变
     */
    public void trackChanged(TrackInfo trackInfo) {
        if(mBitrateRadioGroup != null && mBitrateRadioGroup.getChildCount() > 0 && trackInfo != null){
            RadioButton mRadioButton = (RadioButton) mBitrateRadioGroup.getChildAt(0);
            mRadioButton.setText(getString(R.string.auto_bitrate)+"(" + trackInfo.getVideoBitrate() + ")");
        }
    }
}
