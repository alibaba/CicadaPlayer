package com.cicada.player.utils.ass;

import java.util.Map;

public class AssHeader {

    public static enum SubtitleType {SubtitleTypeUnknown, SubtitleTypeAss, SubtitleTypeSsa}

    public Map<String, AssStyle> mStyles = null;
    public SubtitleType mType = SubtitleType.SubtitleTypeUnknown;

    public int mPlayResX = 0;
    public int mPlayResY = 0;
    public double mTimer = 0;
    public int mWrapStyle = 0;
    public int mScaledBorderAndShadow = 0;
    public String mStyleFormat;
    public String mEventFormat;


    private void setStyles(Object styles) {
        mStyles = (Map<String, AssStyle>) styles;
    }

    private Object getStyles() {
        return mStyles;
    }

    private void setType(int type) {
        if (type == 0) {
            mType = SubtitleType.SubtitleTypeUnknown;
        } else if (type == 1) {
            mType = SubtitleType.SubtitleTypeAss;
        } else if (type == 2) {
            mType = SubtitleType.SubtitleTypeSsa;
        } else {

        }
    }

    private int getType() {
        if (mType == SubtitleType.SubtitleTypeUnknown) {
            return 0;
        } else if (mType == SubtitleType.SubtitleTypeAss) {
            return 1;
        } else if (mType == SubtitleType.SubtitleTypeSsa) {
            return 2;
        } else {
            return 0;
        }
    }
}
