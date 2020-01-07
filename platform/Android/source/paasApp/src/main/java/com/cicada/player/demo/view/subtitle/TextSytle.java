package com.cicada.player.demo.view.subtitle;

import android.graphics.Color;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.AbsoluteSizeSpan;
import android.text.style.ForegroundColorSpan;

import java.util.Map;

import static com.cicada.player.demo.view.subtitle.SubtitleView.EXTRA_COLOR__STRING;
import static com.cicada.player.demo.view.subtitle.SubtitleView.EXTRA_SIZE_PX__INT;


/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class TextSytle {

    public static void setTextColor(SpannableStringBuilder builder, Map<String, Object> extraInfo, String defaultColor) {
        String color = defaultColor;
        if (extraInfo != null && extraInfo.containsKey(EXTRA_COLOR__STRING)) {
            color = (String) extraInfo.get(EXTRA_COLOR__STRING);
        }

        ForegroundColorSpan span = new ForegroundColorSpan(Color.parseColor(color));
        builder.setSpan(span, 0, builder.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
    }

    public static void setTextSize(SpannableStringBuilder builder, Map<String, Object> extraInfo, int defaultSize) {
        int size = defaultSize;
        if (extraInfo != null && extraInfo.containsKey(EXTRA_SIZE_PX__INT)) {
            size = (int) extraInfo.get(EXTRA_SIZE_PX__INT);
        }

        AbsoluteSizeSpan span = new AbsoluteSizeSpan(size);
        builder.setSpan(span, 0, builder.length(), Spannable.SPAN_INCLUSIVE_EXCLUSIVE);
    }
}
