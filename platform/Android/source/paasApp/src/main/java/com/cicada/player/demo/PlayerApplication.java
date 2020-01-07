package com.cicada.player.demo;

import android.support.multidex.MultiDexApplication;

import com.cicada.player.demo.util.SharedPreferenceUtils;


public class PlayerApplication extends MultiDexApplication{

    @Override
    public void onCreate() {
        super.onCreate();
        SharedPreferenceUtils.init(getApplicationContext());
    }
}
