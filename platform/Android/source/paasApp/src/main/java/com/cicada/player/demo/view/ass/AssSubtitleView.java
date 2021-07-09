package com.cicada.player.demo.view.ass;

import android.content.Context;
import android.graphics.Typeface;
import android.util.AttributeSet;
import android.view.View;
import android.widget.RelativeLayout;

import com.cicada.player.demo.R;
import com.cicada.player.utils.ass.AssResolver;

import java.util.HashMap;
import java.util.Map;

public class AssSubtitleView extends RelativeLayout {

    private Map<Long, View> mAssSubtitleView = new HashMap<>();
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

    private void init(Context context){
        mAssResolver = new AssResolver(context);
    }

    public void setFontTypeFace(Map<String,Typeface> typefaceMap){
        mAssResolver.setFontTypeMap(typefaceMap);
    }

    //resolve header
    public void setAssHeader(String header){
        mAssResolver.setAssHeaders(header);
    }

    //show and resolve content
    public void show(long id,String content){
        View view = mAssResolver.setAssDialog(content);
        if(view != null){
            addView(view);
            mAssSubtitleView.put(id,view);
        }
        invalidate();
    }


    public void dismiss(long id){
        View remove = mAssSubtitleView.remove(id);
        if(remove != null){
            removeView(remove);
            mAssResolver.dismiss(remove);
        }

    }

    public void destroy(){
        if(mAssResolver != null){
            mAssResolver.destroy();
        }
    }
}
