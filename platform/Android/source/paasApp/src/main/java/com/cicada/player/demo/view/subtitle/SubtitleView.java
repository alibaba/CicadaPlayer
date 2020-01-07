package com.cicada.player.demo.view.subtitle;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.text.Html;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cicada.player.demo.R;

import java.util.HashMap;
import java.util.Map;

import static com.cicada.player.demo.view.subtitle.LocationStyle.Location_Bottom;
import static com.cicada.player.demo.view.subtitle.LocationStyle.Location_CenterH;


/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class SubtitleView extends RelativeLayout {

    private static final String TAG = SubtitleView.class.getSimpleName();

    public static class Subtitle {
        public String              id;
        public String              content;
        public Map<String, Object> extraInfo;//额外的信息，比如位置，文字颜色之类的
    }

    public static final String EXTRA_COLOR__STRING = "extra_color";
    public static final String EXTRA_LOCATION__INT = "extra_location";
    public static final String EXTRA_GRAVITY__ENUM = "extra_gravity";
    public static final String EXTRA_SIZE_PX__INT  = "extra_size_px";


    private TextViewPool mTextViewPool;

    private Map<String, TextView> mSubtitleView = new HashMap<String, TextView>();

    public SubtitleView(@NonNull Context context) {
        super(context);
        init();
    }

    public SubtitleView(@NonNull Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }


    public SubtitleView(@NonNull Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init() {
        mTextViewPool = new TextViewPool(getContext());
        setDefaultValue(new DefaultValueBuilder());
    }


    public void show(Subtitle subtitle) {

        CharSequence charSequence = getFinalText(subtitle);
        LayoutParams params       = getFinalParam(subtitle);
        TextView     textView     = getFinalTextView(subtitle);
        textView.setLayoutParams(params);
        textView.setText(charSequence);
        textView.setPadding(10,10,10,10);
        textView.setBackgroundResource(R.drawable.cicada_subtitle_view_bg);

        ViewParent viewParent = textView.getParent();
        if (viewParent != null) {
            ((ViewGroup) viewParent).removeView(textView);
        }
        this.addView(textView);

        mSubtitleView.put(subtitle.id, textView);
    }

    private TextView getFinalTextView(Subtitle subtitle) {
        TextView textView = mTextViewPool.obtain();
        do {
            Map<String, Object> extraInfo = subtitle.extraInfo;
            if (extraInfo == null) {
                textView.setGravity(Gravity.CENTER);//默认文字居中
                break;
            }

            if (extraInfo.containsKey(EXTRA_GRAVITY__ENUM)) {

            }

        } while (false);

        return textView;
    }

    private LayoutParams getFinalParam(Subtitle subtitle) {
        LayoutParams params = new LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);

        Map<String, Object> extraInfo = subtitle.extraInfo;
        LocationStyle.setLocation(params, extraInfo, mDefaultLocation);

        return params;
    }


    private SpannableStringBuilder getFinalText(Subtitle subtitle) {
        if (subtitle == null || subtitle.content == null) {
            return new SpannableStringBuilder("");
        }

        String content = subtitle.content;
        //换行问题解决
        content = content.replace("\n", "<br>");

        Spanned                spanned   = Html.fromHtml(content);
        SpannableStringBuilder builder   = new SpannableStringBuilder(spanned);
        Map<String, Object>    extraInfo = subtitle.extraInfo;

        TextSytle.setTextColor(builder, extraInfo, mDefaultColor);
        TextSytle.setTextSize(builder, extraInfo, mDefaultSize);

        return builder;
    }


    public void dismiss(String id) {
        TextView textView = mSubtitleView.remove(id);
        mTextViewPool.recycle(textView);
    }

    private int    mDefaultLocation;
    private float  mDefaultPercent;
    private int    mDefaultSize;
    private String mDefaultColor;

    public void setDefaultValue(DefaultValueBuilder defaultValueBuilder) {
        mDefaultLocation = defaultValueBuilder.mLocation;
        mDefaultPercent = defaultValueBuilder.mTextSizePercent;
        mDefaultSize = defaultValueBuilder.mTextSize;
        mDefaultColor = defaultValueBuilder.mTextColor;
    }


    public static class DefaultValueBuilder {
        int    mLocation        = Location_Bottom | Location_CenterH;
        int    mTextSize        = -1;
        float  mTextSizePercent = 0.08f;
        String mTextColor       = "#FFFFFFFF";

        public DefaultValueBuilder setLocation(int location) {
            mLocation = location;
            return this;
        }

        public DefaultValueBuilder setSize(int textSize) {
            mTextSize = textSize;
            return this;
        }

        public DefaultValueBuilder setSizePercent(float percent) {
            mTextSizePercent = percent;
            return this;
        }

        public DefaultValueBuilder setColor(String color) {
            mTextColor = color;
            return this;
        }
    }

    @Override
    protected void onLayout(boolean changed, int l, int t, int r, int b) {
        super.onLayout(changed, l, t, r, b);
        int height = b - t;
        if (mDefaultSize > 0) {
            return;
        }

        if (mDefaultPercent > 0) {
            mDefaultSize = (int) (mDefaultPercent * height);
        }

        if (mDefaultSize <= 0) {
            mDefaultSize = 20;
        }
    }
}
