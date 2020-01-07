package com.cicada.player.demo.fragment;

import android.support.v4.app.Fragment;

public class BaseFragment extends Fragment {

    protected String mPlaySource;

    public void setPlaySource(String mPlaySource){
        this.mPlaySource = mPlaySource;
    }
}
