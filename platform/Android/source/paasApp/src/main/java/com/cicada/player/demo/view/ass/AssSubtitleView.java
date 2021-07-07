package com.cicada.player.demo.view.ass;

import android.content.Context;
import android.graphics.Typeface;
import android.text.Html;
import android.util.AttributeSet;
import android.view.ViewGroup;
import android.view.ViewParent;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.cicada.player.demo.view.subtitle.TextViewPool;
import com.cicada.player.utils.ass.AssDialogue;
import com.cicada.player.utils.ass.AssHeader;
import com.cicada.player.utils.ass.AssResolver;
import com.cicada.player.utils.ass.AssUtils;

import java.util.HashMap;
import java.util.Map;

public class AssSubtitleView extends RelativeLayout {

    private TextViewPool mTextViewPool;
    private AssHeader assHeader;

    private Map<Long, TextView> mAssSubtitleView = new HashMap<>();
    private Map<String,Typeface> mFontTypeface = new HashMap<>();

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
        mTextViewPool = new TextViewPool(context);
    }

    public void setFontTypeFace(Map<String,Typeface> typefaceMap){
        this.mFontTypeface = typefaceMap;
        AssResolver.setFontTypeMap(mFontTypeface);
    }

    //resolve header
    public void setAssHeader(String header){
        assHeader = AssUtils.parseAssHeader(header);
    }

    //show and resolve content
    public void show(long id,String content){
        AssDialogue assDialogue = AssUtils.parseAssDialogue(assHeader, content);
        TextView textView = mTextViewPool.obtain();
        ViewParent viewParent = textView.getParent();
        if (viewParent != null) {
            ((ViewGroup) viewParent).removeView(textView);
        }

        AssResolver.initTextViewStyle(textView,assHeader,assDialogue);
        String text = AssResolver.parseSubtitleText(assDialogue);
        textView.setText(Html.fromHtml(text));

        addView(textView);
        mAssSubtitleView.put(id,textView);
        invalidate();
    }


    public void dismiss(long id){
        TextView textView = mAssSubtitleView.remove(id);
        mTextViewPool.recycle(textView);
    }
}
