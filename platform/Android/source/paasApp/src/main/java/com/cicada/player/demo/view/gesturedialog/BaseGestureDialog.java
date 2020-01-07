package com.cicada.player.demo.view.gesturedialog;

import android.content.Context;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.PopupWindow;
import android.widget.TextView;

import com.cicada.player.demo.R;



/*
 * Copyright (C) 2010-2018 Alibaba Group Holding Limited.
 */

/**
 * 手势滑动的手势提示框。
 * 其子类有：{@link BrightnessDialog} , {@link SeekDialog} , {@link VolumeDialog}
 */
public class BaseGestureDialog extends PopupWindow {
    //手势文字
    TextView mTextView;
    //手势图片
    ImageView mImageView;
    //对话框的宽高
    private int mDialogWidthAndHeight;

    public BaseGestureDialog(Context context) {
        //使用同一个布局
        LayoutInflater mInflater = (LayoutInflater) context.getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        View view = mInflater.inflate(R.layout.cicada_dialog_gesture, null);
        view.measure(0, 0);
        setContentView(view);

        //找出view
        mTextView = (TextView) view.findViewById(R.id.gesture_text);
        mImageView = (ImageView) view.findViewById(R.id.gesture_image);

        //设置对话框宽高
        mDialogWidthAndHeight = context.getResources().getDimensionPixelSize(R.dimen.cicada_gesture_dialog_size);
        setWidth(mDialogWidthAndHeight);
        setHeight(mDialogWidthAndHeight);
    }

    /**
     * 居中显示对话框
     * @param parent 所属的父界面
     */
    public void show(View parent) {

        int[] location = new int[2];
        parent.getLocationOnScreen(location);
        //保证显示居中
        int x = location[0] + (parent.getRight() - parent.getLeft() - mDialogWidthAndHeight) / 2;
        int y = location[1] + (parent.getBottom() - parent.getTop() - mDialogWidthAndHeight) / 2;

        showAtLocation(parent, Gravity.TOP | Gravity.LEFT, x, y);

    }


}
