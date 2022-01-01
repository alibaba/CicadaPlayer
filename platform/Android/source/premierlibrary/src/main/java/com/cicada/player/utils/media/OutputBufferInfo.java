package com.cicada.player.utils.media;

public class OutputBufferInfo {

    public int type;
    public int index;
    public long pts;
    public int flags;
    public boolean eos;

    public int bufferSize;
    public int bufferOffset;

    public int videoWidth;
    public int videoHeight;
    public int videoStride;
    public int videoSliceHeight;
    public int videoPixelFormat;
    public int videoCropLeft;
    public int videoCropRight;
    public int videoCropTop;
    public int videoCropBottom;

    public int audioChannelCount;
    public int audioChannelMask;
    public int audioSampleRate;
    public int audioFormat;
}


