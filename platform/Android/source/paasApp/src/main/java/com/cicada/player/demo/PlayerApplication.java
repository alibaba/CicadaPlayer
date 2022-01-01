package com.cicada.player.demo;

import android.support.multidex.MultiDexApplication;

import com.aliyun.externalplayer.exo.ExternPlayerExo;
import com.cicada.player.CicadaExternalPlayer;
import com.cicada.player.demo.util.SharedPreferenceUtils;


public class PlayerApplication extends MultiDexApplication{

    @Override
    public void onCreate() {
        super.onCreate();
        SharedPreferenceUtils.init(getApplicationContext());

        CicadaExternalPlayer.registerExternalPlayer(new ExternPlayerExo());
    }
}
