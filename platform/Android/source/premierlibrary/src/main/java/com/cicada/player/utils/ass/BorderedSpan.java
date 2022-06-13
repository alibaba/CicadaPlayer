package com.cicada.player.utils.ass;

import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.text.style.ReplacementSpan;

public class BorderedSpan extends ReplacementSpan {
    final Paint mBorderPaint;
    private BorderStyle mBorderStyle;

    public static class BorderStyle {
        public String fontName;
        public double fontSize;
        public int mPrimaryColour;
        public int mSecondaryColour;
        public boolean mBold;
        public boolean mItalic;
        public boolean mUnderline;
        public boolean mStrikeOut;
        public int mOutlineColour;
        public double mOutlineWidth;
        public double mShadowWidth;
        public int mShadowColor;
    }

    public BorderedSpan(BorderStyle borderStyle) {
        mBorderStyle = borderStyle;
        mBorderPaint = new Paint();
        mBorderPaint.setStyle(Paint.Style.STROKE);
        mBorderPaint.setAntiAlias(true);
    }

    private void fillPainStyle(Paint paint) {
        Typeface typeface = Typeface.create(mBorderStyle.fontName, Typeface.NORMAL);
        paint.setTypeface(typeface);
        paint.setTextSize((float) mBorderStyle.fontSize);
        paint.setColor(mBorderStyle.mPrimaryColour);
//TODO        public int mSecondaryColour;
        paint.setUnderlineText(mBorderStyle.mUnderline);
        paint.setStrikeThruText(mBorderStyle.mStrikeOut);
        paint.setFakeBoldText(mBorderStyle.mBold);
        paint.setTextSkewX(mBorderStyle.mItalic ? -0.25f : 0);
        paint.setShadowLayer(0, (float) mBorderStyle.mShadowWidth, (float) mBorderStyle.mShadowWidth, mBorderStyle.mShadowColor);
    }

    @Override
    public int getSize(Paint paint, CharSequence text, int start, int end, Paint.FontMetricsInt fm) {
        fillPainStyle(paint);
        return (int) paint.measureText(text, start, end);
    }

    @Override
    public void draw(Canvas canvas, CharSequence text, int start, int end, float x, int top, int y, int bottom, Paint paint) {

        if (mBorderStyle.mOutlineWidth > 0) {
            fillPainStyle(mBorderPaint);
            mBorderPaint.setStrokeWidth((float) mBorderStyle.mOutlineWidth);
            mBorderPaint.setColor(mBorderStyle.mOutlineColour);
            canvas.drawText(text, start, end, x, y, mBorderPaint);

            fillPainStyle(paint);
        }

        canvas.drawText(text, start, end, x, y, paint);
    }
}