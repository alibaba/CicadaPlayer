package com.cicada.player.demo.listener;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
/**
 * 设置固定竖屏模式
 */
public interface LockPortraitListener {
    public static final int FIX_MODE_SMALL = 1;
    public static final int FIX_MODE_FULL = 2;

    void onLockScreenMode(int screenMode);

}