package com.cicada.player.demo.view.subtitle;

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
    private List<TextView> idelTextViewList = new ArrayList<TextView>();
    private List<TextView> busyTextViewList = new ArrayList<TextView>();

    public TextViewPool(Context context) {
        mContext = context;
    }

    public TextView obtain() {
        TextView textView = null;
        if (idelTextViewList.isEmpty()) {
            textView = new TextView(mContext);
        } else {
            textView = idelTextViewList.get(0);
        }
        busyTextViewList.add(textView);

        return textView;
    }

    public void recycle(TextView textView) {
        if (textView == null) {
            return;
        }

        ViewGroup parent = (ViewGroup) textView.getParent();
        if(parent != null) {
            parent.removeView(textView);
        }

        busyTextViewList.remove(textView);
        idelTextViewList.add(textView);

    }

}
