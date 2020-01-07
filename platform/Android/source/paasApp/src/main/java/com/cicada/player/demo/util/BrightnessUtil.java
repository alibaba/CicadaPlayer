package com.cicada.player.demo.util;

import android.app.Activity;
import android.view.Window;
import android.view.WindowManager;

import com.cicada.player.demo.util.VcPlayerLog;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */

public class BrightnessUtil {


    public static void setScreenBrightness(Activity mContext, int brightness) {


        //优先使用activity的设置，无需SETTINGS权限。
        VcPlayerLog.d("Player", "setScreenBrightness mContext instanceof Activity brightness = " + brightness);

        //三星手机亮度为0就给你锁屏了。。。这个是适配的bug。。。
        if (brightness > 0) {
            Window localWindow = ((Activity) mContext).getWindow();
            WindowManager.LayoutParams localLayoutParams = localWindow.getAttributes();
            localLayoutParams.screenBrightness = brightness / 100.0F;
            localWindow.setAttributes(localLayoutParams);
        }

    }

    public static int getScreenBrightness(Activity mContext) {

        //优先使用activity的设置，无需SETTINGS权限。
        Window localWindow = ((Activity) mContext).getWindow();
        WindowManager.LayoutParams localLayoutParams = localWindow.getAttributes();

        float screenBrightness = localLayoutParams.screenBrightness;
        if (screenBrightness > 1) {
            screenBrightness = 1;
        } else if (screenBrightness < 0.1) {
            screenBrightness = 0.1f;
        }
        VcPlayerLog.d("Player", "getActivityBrightness layoutParams.screenBrightness = " + screenBrightness);
        return (int) (screenBrightness * 100);

    }

}
