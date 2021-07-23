package com.cicada.player.utils.ass;

import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.text.TextPaint;
import android.util.AttributeSet;
import android.widget.TextView;

public class AssTextView extends TextView {

    public AssTextView(Context context) {
        super(context);
    }

    public AssTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public AssTextView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    private String mContent = null;

    public void setContent(String content) {
        mContent = content;
    }

    public String getContent() {
        return mContent;
    }
}
