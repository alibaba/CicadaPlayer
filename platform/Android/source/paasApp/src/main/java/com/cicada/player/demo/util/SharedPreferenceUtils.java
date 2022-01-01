package com.cicada.player.demo.util;

import android.content.Context;
import android.content.SharedPreferences;

public class SharedPreferenceUtils {

    public static final String CICADA_PLAYER_HARDWARE_DECODER = "cicada_player_hardware_decoder";
    public static final String SELECTED_CICADA_PLAYER = "selected_cicada_player";
    public static final String SELECTED_PLAYER_NAME = "selected_player_type";

    private static SharedPreferences mInstance = null;

    private SharedPreferenceUtils(){

    }

    public static SharedPreferences init(Context context){
        if(context == null){
            return null;
        }
        if(mInstance == null){
            synchronized (SharedPreferenceUtils.class){
                if(mInstance == null){
                    mInstance = context.getSharedPreferences("cicada_player",Context.MODE_PRIVATE);
                }
            }
        }
        return mInstance;
    }

    public static void putBooleanExtra(String key,boolean value){
        if(mInstance == null){
            return ;
        }
        SharedPreferences.Editor edit = mInstance.edit();
        edit.putBoolean(key,value);
        edit.commit();
    }

    public static boolean getBooleanExtra(String key){
        if(mInstance == null){
            return false;
        }
        return mInstance.getBoolean(key, true);
    }

    public static void putStringExtra(String key,String value){
        if(mInstance == null){
            return ;
        }
        SharedPreferences.Editor edit = mInstance.edit();
        edit.putString(key,value);
        edit.commit();
    }

    public static String getStringExtra(String key){
        if(mInstance == null){
            return "";
        }
        return mInstance.getString(key, "");
    }
}
