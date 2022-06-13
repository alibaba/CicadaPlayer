package com.cicada.player.utils.ass;

import android.content.Context;
import android.view.ViewGroup;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class TextViewPool {

    private Context mContext;
    private List<AssTextView> idelTextViewList = new ArrayList<AssTextView>();
    private List<AssTextView> busyTextViewList = new ArrayList<AssTextView>();

    public TextViewPool(Context context) {
        mContext = context;
    }

    public AssTextView obtain() {
        AssTextView textView = null;
        if (idelTextViewList.isEmpty()) {
            textView = new AssTextView(mContext);
        } else {
            textView = idelTextViewList.get(0);
            idelTextViewList.remove(textView);
        }
        busyTextViewList.add(textView);

        return textView;
    }

    public void recycle(AssTextView textView) {
        if (textView == null) {
            return;
        }

        ViewGroup parent = (ViewGroup) textView.getParent();
        if(parent != null) {
            parent.removeView(textView);
        }
        textView.setText("");
        busyTextViewList.remove(textView);
        idelTextViewList.add(textView);

    }

}
