
#ifndef CicadaOCHelper_H
#define CicadaOCHelper_H

#import "CicadaPlayer.h"
#include "native_cicada_player_def.h"

class CicadaOCHelper {
public:
    CicadaOCHelper(CicadaPlayer * player):mPlayer(player){}
    void getListener(playerListener &listener);

    void setDelegate(__weak id<CicadaDelegate> innerDelegate) {
        mInnerDelegate = innerDelegate;
    }

    static CicadaTrackInfo* getCicadaTrackInfo(const StreamInfo *info);
    static CicadaImage * convertBitmapRGBA8ToUIImage(unsigned char *buffer, int width, int height);

protected:
    static CicadaPlayer * getOCPlayer(void *userData);
    static id<CicadaDelegate> getDelegate(void *userData);

    static void onError(int64_t code, const void *msg, /*void *extra, */void *userData);

    static void onEvent(int64_t code, const void *msg, /*void *extra, */void *userData);

    static void onVideoRendered(int64_t theTimeMs, int64_t thePts, void *userData);

    static void onPrepared(void *userData);

    static void onCompletion(void *userData);

    static void onCircleStart(void *userData);

    static void onAutoPlayStart(void *userData);

    static void onFirstFrameShow(void *userData);

    static void onVideoSizeChanged(int64_t width, int64_t height, void *userData);

    static void onCurrentPositionUpdate(int64_t position, void *userData);

    static void onBufferPositionUpdate(int64_t position, void *userData);

    static void onLoadingStart(void *userData);

    static void onLoadingProgress(int64_t percent, void *userData);

    static void onLoadingEnd(void *userData);

    static void onSeekEnd(int64_t seekInCache, void *userData);

    static void onStreamInfoGet(int64_t count, const void *infos, void *userData);

    static void
    onSwitchStreamSuccess(int64_t type, const void *item, void *userData);

    static void
    onPlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData);

    static void
    onShowSubtitle(int64_t index, int64_t size, const void *content,/*void *extra,*/
                       void *userData);

    static void onHideSubtitle(int64_t index, int64_t size, const void *content,void *userData);

    static void onSubtitleExtAdd(int64_t index, const void *url, void *userData);

    static void
    onCaptureScreen(int64_t width, int64_t height, const void *buffer,/*void *extra,*/
                        void *userData);

    static CicadaStatus mapStatus(int64_t status);

private:
    __weak CicadaPlayer * mPlayer = nullptr;
    __weak id<CicadaDelegate> mInnerDelegate = nil;
};


#endif //CicadaOCHelper_H
