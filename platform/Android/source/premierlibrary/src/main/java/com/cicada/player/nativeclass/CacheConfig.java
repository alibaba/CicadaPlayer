package com.cicada.player.nativeclass;

/*
 * Copyright (C) 2010-2017 Alibaba Group Holding Limited.
 */
public class CacheConfig {

    /**
     * 是否开启缓存功能.true:开启。默认关闭。
     */
    /****
     * Indicate whether content caching is enabled. Value true indicates that content caching is enabled. The default is disabled.
     */
    public boolean mEnable = false;
    /**
     * 缓存目录
     */
    /****
     * The directory of the cache.
     */
    public String mDir;

    /**
     * 缓存目录的最大占用空间。如果超过，则删除最旧的文件。单位：MB
     */
    /****
     * The maximum cache size. If the size of the files need to be cached exceeds the maximum cache size, the earliest files are deleted. Unit: MB.
     */
    public int mMaxSizeMB;

    /**
     * 设置能够缓存的单个文件的最大时长。如果文件的时长超过此时长，则不会缓存。单位：秒。
     */
    /****
     * The maximum size of a single file that can be cached. Files that fail to match this limit are not cached. Unit: seconds.
     */
    public long mMaxDurationS;
}
