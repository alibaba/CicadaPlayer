package com.cicada.player.nativeclass;

import java.util.HashMap;
import java.util.Map;

public class Options {

    private static final int FLAG_APPEND = 1;
    private static final int FLAG_REPLACE = 2;

    private Map<String, String> mOptions = new HashMap<>();

    public String get(String key) {
        return mOptions.get(key);
    }

    public boolean set(String key, String value, int flag) {
        if (mOptions.containsKey(key)) {
            if (flag == FLAG_APPEND) {
                mOptions.put(key, mOptions.get(key) + value);
            } else if (flag == FLAG_REPLACE) {
                mOptions.put(key, value);
            } else {
                return false;
            }
        } else {
            mOptions.put(key, value);
        }
        return true;
    }

    public void reset() {
        mOptions.clear();
    }
}
