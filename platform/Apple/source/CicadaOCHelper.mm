
#include "CicadaOCHelper.h"
#include <EventCodeMap.h>
#include <ErrorCodeMap.h>

CicadaImage * CicadaOCHelper::convertBitmapRGBA8ToUIImage(unsigned char *buffer, int width, int height) {
    size_t bufferLength = width * height * 4;
    CGDataProviderRef provider = CGDataProviderCreateWithData(NULL, buffer, bufferLength, NULL);
    size_t bitsPerComponent = 8;
    size_t bitsPerPixel = 32;
    size_t bytesPerRow = 4 * width;

    CGColorSpaceRef colorSpaceRef = CGColorSpaceCreateDeviceRGB();
    if(colorSpaceRef == NULL) {
        NSLog(@"Error allocating color space");
        CGDataProviderRelease(provider);
        return nil;
    }

    CGBitmapInfo bitmapInfo = kCGBitmapByteOrderDefault;
    CGColorRenderingIntent renderingIntent = kCGRenderingIntentDefault;

    CGImageRef iref = CGImageCreate(width,
                                    height,
                                    bitsPerComponent,
                                    bitsPerPixel,
                                    bytesPerRow,
                                    colorSpaceRef,
                                    bitmapInfo,
                                    provider,    // data provider
                                    NULL,        // decode
                                    YES,            // should interpolate
                                    renderingIntent);

    uint32_t* pixels = (uint32_t*)malloc(bufferLength);

    if (pixels == NULL) {
        NSLog(@"Error: Memory not allocated for bitmap");
        CGDataProviderRelease(provider);
        CGColorSpaceRelease(colorSpaceRef);
        CGImageRelease(iref);
        return nil;
    }

    CGContextRef context = CGBitmapContextCreate(pixels,
                                                 width,
                                                 height,
                                                 bitsPerComponent,
                                                 bytesPerRow,
                                                 colorSpaceRef,
                                                 kCGImageAlphaPremultipliedLast);

    CicadaImage *image = nil;
    if (context) {

        CGContextDrawImage(context, CGRectMake(0.0f, 0.0f, width, height), iref);

        CGImageRef imageRef = CGBitmapContextCreateImage(context);

#if TARGET_OS_OSX
        NSSize size;
        size.height = height;
        size.width = width;
        image = [[NSImage alloc] initWithCGImage:imageRef size:size];
#else
        // Support both iPad 3.2 and iPhone 4 Retina displays with the correct scale
        if([UIImage respondsToSelector:@selector(imageWithCGImage:scale:orientation:)]) {
            float scale = [[UIScreen mainScreen] scale];
            image = [UIImage imageWithCGImage:imageRef scale:scale orientation:UIImageOrientationUp];
        } else {
            image = [UIImage imageWithCGImage:imageRef];
        }
#endif
        CGImageRelease(imageRef);
        CGContextRelease(context);
    }

    CGColorSpaceRelease(colorSpaceRef);
    CGImageRelease(iref);
    CGDataProviderRelease(provider);

    if (pixels) {
        free(pixels);
    }

    return image;
}

CicadaTrackInfo* CicadaOCHelper::getCicadaTrackInfo(const StreamInfo *info)
{
    if (nullptr == info) {
        return nil;
    }
    CicadaTrackInfo* trackInfo = [[CicadaTrackInfo alloc] init];

    trackInfo.trackType = static_cast<CicadaTrackType>(info->type);
    trackInfo.trackIndex = info->streamIndex;
    trackInfo.videoWidth = info->videoWidth;
    trackInfo.videoHeight = info->videoHeight;
    trackInfo.trackBitrate = info->videoBandwidth;
    trackInfo.audioChannels = info->nChannels;
    trackInfo.audioSamplerate = info->sampleRate;
    trackInfo.audioSampleFormat = info->sampleFormat;

    if (info->description) {
        trackInfo.description = [NSString stringWithUTF8String:info->description];
    }
    if (info->audioLang) {
        trackInfo.audioLanguage = [NSString stringWithUTF8String:info->audioLang];
    }
    if (info->subtitleLang) {
        trackInfo.subtitleLanguage = [NSString stringWithUTF8String:info->subtitleLang];
    }

    return trackInfo;
}

CicadaStatus CicadaOCHelper::mapStatus(int64_t status) {
    if (status == PLAYER_IDLE) {
        return CicadaStatusIdle;
    } else if (status == PLAYER_INITIALZED) {
        return CicadaStatusInitialzed;
    } else if (status == PLAYER_PREPARED) {
        return CicadaStatusPrepared;
    } else if (status == PLAYER_PLAYING) {
        return CicadaStatusStarted;
    } else if (status == PLAYER_PAUSED) {
        return CicadaStatusPaused;
    } else if (status == PLAYER_STOPPED) {
        return CicadaStatusStopped;
    } else if (status == PLAYER_COMPLETION) {
        return CicadaStatusCompletion;
    } else if (status == PLAYER_ERROR) {
        return CicadaStatusError;
    } else {
        return CicadaStatusMax;
    }
}

void CicadaOCHelper::getListener(playerListener &listener)
{
    listener.userData = this;
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
    listener.SubtitleExtAdd = onSubtitleExtAdd;
    listener.StatusChanged = onPlayerStatusChanged;
    listener.CaptureScreen = onCaptureScreen;
    listener.SubtitleShow = onShowSubtitle;
    listener.SubtitleHide = onHideSubtitle;
    listener.VideoRendered = onVideoRendered;
    listener.EventCallback = onEvent;
    listener.ErrorCallback = onError;
    listener.Prepared = onPrepared;
    listener.Completion = onCompletion;
}

CicadaPlayer * CicadaOCHelper::getOCPlayer(void *userData)
{
    CicadaOCHelper *helper = (CicadaOCHelper *)userData;
    if (nullptr == helper) {
        return nil;
    }
    return helper->mPlayer;
}

id<CicadaDelegate> CicadaOCHelper::getDelegate(void *userData)
{
    CicadaOCHelper *helper = (CicadaOCHelper *)userData;
    if (nullptr == helper) {
        return nil;
    }
    return helper->mInnerDelegate;
}

void CicadaOCHelper::onPrepared(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    __weak id<CicadaDelegate> theDelegate = getDelegate(userData);

    dispatch_async(dispatch_get_main_queue(), ^{
        player.duration = 0; // setDuration will overwrite the value
        if (theDelegate && [theDelegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
            [theDelegate onPlayerEvent:player eventType:CicadaEventPrepareDone];
        }
        
        if (player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
            [player.delegate onPlayerEvent:player eventType:CicadaEventPrepareDone];
        }
    });
}

void CicadaOCHelper::onCompletion(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{

            [player.delegate onPlayerEvent:player eventType:CicadaEventCompletion];
        });
    }
}

void CicadaOCHelper::onFirstFrameShow(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventFirstRenderedStart];
        });
    }
}

void CicadaOCHelper::onCircleStart(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventLoopingStart];
        });
    }
}

void CicadaOCHelper::onError(int64_t code, const void *msg, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    NSString* errorStr = [[NSString alloc] initWithUTF8String:(const char *)msg];
    if(player.delegate && [player.delegate respondsToSelector:@selector(onError:errorModel:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            CicadaErrorModel* model = [[CicadaErrorModel alloc] init];
            model.code = (CicadaErrorCode)ErrorCodeMap::getInstance()->getValue(static_cast<int>(code));
            model.message = errorStr;
            [player.delegate onError:player errorModel:model];
        });
    }
}

void CicadaOCHelper::onVideoSizeChanged(int64_t width, int64_t height, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        [player setWidth:(int)width];
        [player setHeight:(int)height];
        //TODO: rotation value
        if (player.delegate && [player.delegate respondsToSelector:@selector(onVideoSizeChanged:width:height:rotation:)]){
            [player.delegate onVideoSizeChanged:player width:(int)width height:(int)height rotation:0.0f];
        }
    });
}

void CicadaOCHelper::onCurrentPositionUpdate(int64_t position, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    __weak id<CicadaDelegate> theDelegate = getDelegate(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        [player setCurrentPosition:position];
        if (player.delegate && [player.delegate respondsToSelector:@selector(onCurrentPositionUpdate:position:)]) {
            [player.delegate onCurrentPositionUpdate:player position:position];
        }
        if (theDelegate && [theDelegate respondsToSelector:@selector(onCurrentPositionUpdate:position:)]) {
            [theDelegate onCurrentPositionUpdate:player position:position];
        }
    });
}

void CicadaOCHelper::onBufferPositionUpdate(int64_t position, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        [player setBufferedPosition:position];
        if (player.delegate && [player.delegate respondsToSelector:@selector(onBufferedPositionUpdate:position:)]) {
            [player.delegate onBufferedPositionUpdate:player position:position];
        }
    });
}

void CicadaOCHelper::onLoadingStart(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventLoadingStart];
        });
    }
}

void CicadaOCHelper::onLoadingEnd(void *userData)     {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventLoadingEnd];
        });
    }
}

void CicadaOCHelper::onLoadingProgress(int64_t percent, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if(player.delegate && [player.delegate respondsToSelector:@selector(onLoadingProgress:progress:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onLoadingProgress:player progress:percent];
        });
    }
}

void CicadaOCHelper::onShowSubtitle(int64_t index, int64_t size, const void *data, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);

    if (nullptr == data) return;
    IAFPacket *packet = (IAFPacket *) (data);
    int subtitleIndex = packet->getInfo().streamIndex;
    int64_t pts = packet->getInfo().pts;
    NSData* stringData = [[NSData alloc] initWithBytes:packet->getData() length:(unsigned int)packet->getSize()];
    NSString* str = [[NSString alloc] initWithData:stringData encoding:NSUTF8StringEncoding];

    if(player.delegate && [player.delegate respondsToSelector:@selector(onSubtitleShow:trackIndex:subtitleID:subtitle:)]){
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onSubtitleShow:player trackIndex:(int)subtitleIndex subtitleID:pts subtitle:str];
        });
    }
}

void
CicadaOCHelper::onCaptureScreen(int64_t width, int64_t height, const void *buffer, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (nullptr == buffer) {
        return;
    }
    CicadaImage* image = convertBitmapRGBA8ToUIImage((unsigned char *)buffer, static_cast<int>(width), static_cast<int>(height));

    if(player.delegate && [player.delegate respondsToSelector:@selector(onCaptureScreen:image:)]){
        dispatch_async(dispatch_get_main_queue(), ^{

            [player.delegate onCaptureScreen:player image:image];
        });
    }
}

void CicadaOCHelper::onHideSubtitle(int64_t index, int64_t size, const void *data, void *userData)
{
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (nullptr == data) return;
    IAFPacket *packet = (IAFPacket *) (data);
    int subtitleIndex = packet->getInfo().streamIndex;
    int64_t pts = packet->getInfo().pts;

    if (player.delegate && [player.delegate respondsToSelector:@selector(onSubtitleHide:trackIndex:subtitleID:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onSubtitleHide:player trackIndex:(int)subtitleIndex subtitleID:pts];
        });
    }
}

void CicadaOCHelper::onSubtitleExtAdd(int64_t index, const void *url, void *userData)
{
    __weak CicadaPlayer * player = getOCPlayer(userData);
    NSString *str = [NSString stringWithUTF8String:(const char *)url];

    if (player.delegate && [player.delegate respondsToSelector:@selector(onSubtitleExtAdded:trackIndex:URL:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onSubtitleExtAdded:player trackIndex:(int)index URL:str];
        });
    }
}

void
CicadaOCHelper::onSwitchStreamSuccess(int64_t type, const void *item, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    const auto *newStream = static_cast<const StreamInfo *>(item);
    int streamIndex = newStream->streamIndex;
    if (player.delegate && [player.delegate respondsToSelector:@selector(onTrackChanged:info:)]){
        dispatch_async(dispatch_get_main_queue(), ^{

            CicadaMediaInfo* mediaInfo = [player getMediaInfo];
            for (CicadaTrackInfo* track in mediaInfo.tracks) {
                if (streamIndex == track.trackIndex) {
                    if (track.trackType == type) {
                        [player.delegate onTrackChanged:player info:track];
                        break;
                    }
                }
            }
        });
    }
}

void CicadaOCHelper::onStreamInfoGet(int64_t count, const void *infos, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    CicadaMediaInfo* mediaInfo = [player getMediaInfo];

    if (count > 0 && (nullptr != infos)) {
        auto ** streamInfos = (StreamInfo **)infos;
        NSMutableArray* tracks = [[NSMutableArray alloc] init];
        for (int i = 0; i < count; ++i) {
            CicadaTrackInfo* trackInfo = getCicadaTrackInfo(streamInfos[i]);
            [tracks addObject:trackInfo];
        }

        mediaInfo.tracks = tracks;

        if ([tracks count] > 0) {
            if(player.delegate && [player.delegate respondsToSelector:@selector(onTrackReady:info:)]){
                dispatch_async(dispatch_get_main_queue(), ^{

                    [player.delegate onTrackReady:player info:tracks];
                });
            }
        }
    }
}

void CicadaOCHelper::onSeekEnd(int64_t seekInCache, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (player && player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onPlayerEvent:player eventType:CicadaEventSeekEnd];
        });
    }
}

void CicadaOCHelper::onAutoPlayStart(void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    if (player && player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventType:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{

            [player.delegate onPlayerEvent:player eventType:CicadaEventAutoPlayStart];
        });
    }
}

void CicadaOCHelper::onPlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    __weak id<CicadaDelegate> theDelegate = getDelegate(userData);
    dispatch_async(dispatch_get_main_queue(), ^{
        if (theDelegate && [theDelegate respondsToSelector:@selector(onPlayerStatusChanged:oldStatus:newStatus:)]) {
            [theDelegate onPlayerStatusChanged:player oldStatus:mapStatus(oldStatus) newStatus:mapStatus(newStatus)];
        }

        if (player && player.delegate && [player.delegate respondsToSelector:@selector(onPlayerStatusChanged:oldStatus:newStatus:)]) {
            [player.delegate onPlayerStatusChanged:player oldStatus:mapStatus(oldStatus) newStatus:mapStatus(newStatus)];
        }
    });
}

void CicadaOCHelper::onEvent(int64_t code, const void *msg, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);
    NSString* str = @"";
    if (msg != nullptr) {
        str = [[NSString alloc] initWithUTF8String:(const char *)msg];
    }

    if (player.delegate && [player.delegate respondsToSelector:@selector(onPlayerEvent:eventWithString:description:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{

            CicadaEventWithString eventCode = (CicadaEventWithString)EventCodeMap::getInstance()->getValue(static_cast<int>(code));
            [player.delegate onPlayerEvent:player eventWithString:(CicadaEventWithString)eventCode description:str];
        });
    }
}

void CicadaOCHelper::onVideoRendered(int64_t theTimeMs, int64_t thePts, void *userData) {
    __weak CicadaPlayer * player = getOCPlayer(userData);

    if (player.delegate && [player.delegate respondsToSelector:@selector(onVideoRendered:timeMs:pts:)]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [player.delegate onVideoRendered:player timeMs:theTimeMs pts:thePts];
        });
    }
}
