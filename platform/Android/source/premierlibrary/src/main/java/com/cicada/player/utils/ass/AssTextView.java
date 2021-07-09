package com.cicada.player.utils.ass;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.util.AttributeSet;
import android.view.View;
import android.widget.TextView;

public class AssTextView extends TextView {

    private Paint mPaint;
    private Path mPath = new Path();

    public AssTextView(Context context) {
        super(context);
        init();
    }

    public AssTextView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public AssTextView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    private void init(){
        setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        mPaint = new Paint(Paint.ANTI_ALIAS_FLAG);

    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
    }


}
