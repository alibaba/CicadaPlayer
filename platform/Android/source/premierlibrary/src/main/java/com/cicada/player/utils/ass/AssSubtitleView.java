package com.cicada.player.utils.ass;

import android.content.Context;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.widget.RelativeLayout;

import java.util.HashMap;
import java.util.Map;

public class AssSubtitleView extends RelativeLayout {

    private Map<Long, AssTextView> mAssSubtitleView = new HashMap<>();
    private AssResolver mAssResolver;

    public AssSubtitleView(Context context) {
        super(context);
        init(context);
    }

    public AssSubtitleView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init(context);
    }

    public AssSubtitleView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init(context);
    }

    private void init(Context context) {
        mAssResolver = new AssResolver(context);
    }

    public void setFontTypeFace(Map<String,Typeface> typefaceMap){
        mAssResolver.setFontTypeMap(typefaceMap);
    }

    //resolve header
    public void setAssHeader(String header) {
        mAssResolver.setAssHeaders(header);
    }

    //show and resolve content
    public synchronized void show(long id, String content) {
        AssTextView view = mAssResolver.setAssDialog(content);

        if (view != null) {
            addView(view);
            mAssSubtitleView.put(id, view);
        }
        invalidate();
    }


    public synchronized void dismiss(long id) {
        AssTextView remove = mAssSubtitleView.remove(id);
        if (remove != null) {
            removeView(remove);
            mAssResolver.dismiss(remove);
        }

    }

    public void destroy() {
        if (mAssResolver != null) {
            mAssResolver.destroy();
        }
    }
}
