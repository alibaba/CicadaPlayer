//
// Created by moqi on 2019/12/4.
//

#include "getListener.h"

static void onCircleStart(void *userData)
{
}

static void onAutoPlayStart(void *userData)
{
}

static void onFirstFrameShow(void *userData)
{
}

static void onVideoSizeChanged(int64_t width, int64_t height, void *userData)
{
    AF_LOGI("AliyunCorePlayer  onVideoSizeChanged  width = %d ,height = %d", width, height);
}

static void onCurrentPositionUpdate(int64_t position, void *userData)
{
}

static void onBufferPositionUpdate(int64_t position, void *userData)
{
}

static void onLoadingStart(void *userData)
{
}

static void onLoadingProgress(int64_t percent, void *userData)
{
}

static void onLoadingEnd(void *userData)
{
}

static void onSeekEnd(int64_t seekInCache, void *userData)
{
}

static void onPlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData)
{
}

static void onCaptureScreen(int64_t width, int64_t height, const void *buffer, void *userData)
{
    if (userData == nullptr) {
        return;
    }
}

static void onSubTitleExtAdd(int64_t index, const void *url, void *userData)
{
}

static void onShowSubtitle(int64_t id, int64_t size, const void *content, void *userData)
{
}

static void onHideSubtitle(int64_t id, int64_t size, const void *content, void *userData)
{
}

static void onEvent(int64_t code, const void *msg, void *userData)
{
}

static void onError(int64_t code, const void *msg, void *userData)
{
}

static void onPrepared(void *userData)
{
}

static void onCompletion(void *userData)
{
}

static void onStreamInfoGet(int64_t count, const void *infos, void *userData)
{
}

static void onSwitchStreamSuccess(int64_t type, const void *item, void *userData)
{
}

playerListener getListener()
{
    playerListener listener{nullptr};
    listener.userData = nullptr;//userData;
    listener.LoopingStart = onCircleStart;
    listener.AutoPlayStart = onAutoPlayStart;
    listener.FirstFrameShow = onFirstFrameShow;
    listener.VideoSizeChanged = onVideoSizeChanged;
    listener.PositionUpdate = onCurrentPositionUpdate;
    listener.BufferPositionUpdate = onBufferPositionUpdate;
    listener.LoadingStart = onLoadingStart;
    listener.LoadingProgress = onLoadingProgress;
    listener.LoadingEnd = onLoadingEnd;
    listener.SeekEnd = onSeekEnd;
    listener.StreamInfoGet = onStreamInfoGet;
    listener.StreamSwitchSuc = onSwitchStreamSuccess;
    listener.StatusChanged = onPlayerStatusChanged;
    listener.CaptureScreen = onCaptureScreen;
    listener.SubtitleShow = onShowSubtitle;
    listener.SubtitleHide = onHideSubtitle;
    listener.EventCallback = onEvent;
    listener.ErrorCallback = onError;
    listener.Prepared = onPrepared;
    listener.Completion = onCompletion;
    listener.SubtitleExtAdd = onSubTitleExtAdd;
}
