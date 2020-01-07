package com.cicada.player.demo.view;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;

import com.cicada.player.demo.R;
import com.cicada.player.demo.util.DensityUtils;

/**
 * data:2019-10-24
 */
public class IndicatorSeekBar extends android.support.v7.widget.AppCompatSeekBar {
    private Paint mPaint = null;

    public IndicatorSeekBar(Context context) {
        super(context);
    }

    public IndicatorSeekBar(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public IndicatorSeekBar(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
    }

    @Override
    protected synchronized void onDraw(Canvas canvas) {
        int height = getHeight();
        int width = getWidth();
        if (mPaint == null){
            mPaint = new Paint();
            mPaint.setColor(getResources().getColor(R.color.cicada_little_download_bg));
        }
        float left = width / 2.0f - DensityUtils.dip2px(getContext(), 1);
        float top =  height / 2.0f - DensityUtils.dip2px(getContext(), 8);
        float right = width / 2.0f + DensityUtils.dip2px(getContext(), 1);
        float bottom = height / 2.0f + DensityUtils.dip2px(getContext(), 8);
        canvas.drawRect(left,top,right,bottom,mPaint);
        super.onDraw(canvas);
    }
}
