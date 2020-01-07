package com.cicada.player.demo.view.subtitle;

import android.widget.RelativeLayout;

import java.util.Map;

import static com.cicada.player.demo.view.subtitle.SubtitleView.EXTRA_LOCATION__INT;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class LocationStyle {

    public static final int Location_Left    = 1;
    public static final int Location_Top     = 2 << 0;
    public static final int Location_Right   = 2 << 1;
    public static final int Location_Bottom  = 2 << 2;
    public static final int Location_CenterH = 2 << 3;
    public static final int Location_CenterV = 2 << 4;
    public static final int Location_Center  = 2 << 5;

    public static void setLocation(RelativeLayout.LayoutParams params, Map<String, Object> extraInfo, int defaultLocation) {
        int location = defaultLocation;
        if (extraInfo != null && extraInfo.containsKey(EXTRA_LOCATION__INT)) {
            location = (int) extraInfo.get(EXTRA_LOCATION__INT);
        }

        if ((location & Location_Bottom) == Location_Bottom) {
            params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
        }

        if ((location & Location_Left) == Location_Left) {
            params.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
        }

        if ((location & Location_Top) == Location_Top) {
            params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
        }

        if ((location & Location_Right) == Location_Right) {
            params.addRule(RelativeLayout.ALIGN_PARENT_RIGHT);
        }

        if ((location & Location_CenterH) == Location_CenterH) {
            params.addRule(RelativeLayout.CENTER_HORIZONTAL);
        }

        if ((location & Location_CenterV) == Location_CenterV) {
            params.addRule(RelativeLayout.CENTER_VERTICAL);
        }

        if ((location & Location_Center) == Location_Center) {
            params.addRule(RelativeLayout.CENTER_IN_PARENT);
        }
    }
}
