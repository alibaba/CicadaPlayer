//
// Created by moqi on 2020/7/20.
//

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif
#include "AppleAVPlayer.h"
#include "AppleAVPlayerLayerProcessor.h"
#include "AppleAVPlayerUtil.h"
#include "AppleAVPlayerHandler.h"

using namespace Cicada;

AppleAVPlayer AppleAVPlayer::se(1);
AppleAVPlayer::AppleAVPlayer()
{
}

AppleAVPlayer::~AppleAVPlayer()
{
    if (mIsDummy) {
        return;
    }
    Stop();
    if (this->parentLayer) {
        CFRelease(this->parentLayer);
    }
    this->parentLayer = nullptr;
}

int AppleAVPlayer::SetListener(const playerListener &Listener)
{
    this->mListener = Listener;
    if (this->playerHandler != NULL) {
        AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
        [playerHandler setmPlayerListener:Listener];
    }
    return 0;
}

void AppleAVPlayer::SetOnRenderCallBack(onRenderFrame cb, void *userData)
{
    
}

void AppleAVPlayer::recheckHander()
{
    if (this->playerHandler == NULL) {
        return;
    }
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    if (this->parentLayer != NULL) {
        playerHandler.parentLayer = (__bridge CALayer *)this->parentLayer;
    }
    if (this->avPlayer != NULL) {
        playerHandler.avplayer = (__bridge AVPlayer *)this->avPlayer;
    }
    [playerHandler setmPlayerListener:this->mListener];
}

void AppleAVPlayer::SetView(void *view)
{
    if (this->parentLayer) {
        AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
        if (this->parentLayer != NULL) {
            [playerHandler removePlayerLayer];
        }
        CFRelease(this->parentLayer);
    }
    this->parentLayer = view;
    if (view) {
        CFRetain(view);
        this->recheckHander();
    }
}

void AppleAVPlayer::SetDataSource(const char *url)
{
//    this->sourceUrl = (char *)url;
    NSString *urlString = [[NSString alloc] initWithUTF8String:url];
    this->sourceUrl = (__bridge_retained void *)urlString;
//    NSLog(@"SetDataSource url : %@", urlString);
    UpdatePlayerStatus(PLAYER_INITIALZED);
}

void AppleAVPlayer::UpdatePlayerStatus(PlayerStatus status)
{
    if (mListener.StatusChanged) {
        mListener.StatusChanged(mStatus, status, mListener.userData);
    }

    mStatus = status;
}

void AppleAVPlayer::Prepare()
{
    NSString *urlString = (__bridge NSString *)this->sourceUrl;
    NSLog(@"Prepare url : %@", urlString);
    NSURL *mediaURL = [NSURL URLWithString:urlString];
    AVURLAsset *asset = [AVURLAsset assetWithURL:mediaURL];
    if (this->resourceLoaderDelegate) {
        [asset.resourceLoader setDelegate:(__bridge id<AVAssetResourceLoaderDelegate>)this->resourceLoaderDelegate queue:dispatch_get_main_queue()];
    }
    AVPlayerItem *item = [AVPlayerItem playerItemWithAsset:asset];
    UpdatePlayerStatus(PLAYER_PREPARING);
    AVPlayer *player = [[AVPlayer alloc] initWithPlayerItem:item];
    this->avPlayer = (__bridge_retained void *)player;
    AppleAVPlayerHandler *playerHandler = [[AppleAVPlayerHandler alloc] init];
    this->playerHandler = (__bridge_retained void *)playerHandler;
    this->recheckHander();
    
    // 初始化轨道信息
    NSArray<AVMediaCharacteristic> *array = [asset availableMediaCharacteristicsWithMediaSelectionOptions];
    NSMutableArray *selectionOptionArray = [NSMutableArray array];
    [array enumerateObjectsUsingBlock:^(AVMediaCharacteristic  _Nonnull obj, NSUInteger idx, BOOL * _Nonnull stop) {
        AVMediaSelectionGroup *mediaGroup = [asset mediaSelectionGroupForMediaCharacteristic:obj];
        [mediaGroup.options enumerateObjectsUsingBlock:^(AVMediaSelectionOption * _Nonnull options, NSUInteger idx, BOOL * _Nonnull stop) {
            [selectionOptionArray addObject:options];
        }];
    }];
    playerHandler.selectionOptionArray = selectionOptionArray;

    int size = (int)selectionOptionArray.count;
    this->mStreamInfos = new StreamInfo *[size];
    [selectionOptionArray enumerateObjectsUsingBlock:^(AVMediaSelectionOption * _Nonnull options, NSUInteger idx, BOOL * _Nonnull stop) {
        auto *info = new StreamInfo();
        info->streamIndex = (int)idx;
        if (options.displayName != nullptr) {
            info->description = strdup((const char *) [options.displayName UTF8String]);
        }

        if ([options.mediaType isEqualToString:AVMediaTypeSubtitle]) {
            info->type = ST_TYPE_SUB;
            if (options.extendedLanguageTag != nullptr) {
                info->subtitleLang = strdup((const char *) [options.extendedLanguageTag UTF8String]);
            }
        } else if ([options.mediaType isEqualToString:AVMediaTypeAudio]) {
            info->type = ST_TYPE_AUDIO;
            if (options.extendedLanguageTag != nullptr) {
                info->audioLang = strdup((const char *) [options.extendedLanguageTag UTF8String]);
            }
        } else if ([options.mediaType isEqualToString:AVMediaTypeVideo]) {
            info->type = ST_TYPE_VIDEO;
        }
        this->mStreamInfos[idx] = info;
    }];
    
    if (this->mListener.StreamInfoGet) {
        this->mListener.StreamInfoGet((int64_t)size, this->mStreamInfos, this->mListener.userData);
    }
    
    if (this->isAutoPlay) {
        [player play];
        UpdatePlayerStatus(PLAYER_PLAYING);
    }
}

void AppleAVPlayer::Start()
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    [player play];
    UpdatePlayerStatus(PLAYER_PLAYING);
}

void AppleAVPlayer::Pause()
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    [player pause];
    UpdatePlayerStatus(PLAYER_PAUSED);
}

StreamType AppleAVPlayer::SwitchStream(int index)
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    NSArray *optionArray = playerHandler.selectionOptionArray;
    AVMediaSelectionOption *option = optionArray[index];
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    AVMediaSelectionGroup *mediaGroup = nil;
    AVAsset *asset = player.currentItem.asset;
    if ([option.mediaType isEqualToString:AVMediaTypeSubtitle]) {
        mediaGroup = [asset mediaSelectionGroupForMediaCharacteristic:AVMediaCharacteristicLegible];
    } else if ([option.mediaType isEqualToString:AVMediaTypeAudio]) {
        mediaGroup = [asset mediaSelectionGroupForMediaCharacteristic:AVMediaCharacteristicAudible];
    } else if ([option.mediaType isEqualToString:AVMediaTypeVideo]) {
        mediaGroup = [asset mediaSelectionGroupForMediaCharacteristic:AVMediaCharacteristicVisual];
    }
    [player.currentItem selectMediaOption:option inMediaSelectionGroup:mediaGroup];

    StreamInfo *info = this->mStreamInfos[index];
    StreamType type = info->type;
    if (this->mListener.StreamSwitchSuc) {
        this->mListener.StreamSwitchSuc(type, info, this->mListener.userData);
    }
    return type;
}

void AppleAVPlayer::SeekTo(int64_t seekPos, bool bAccurate)
{
    if (this->mListener.Seeking) {
        this->mListener.Seeking(1, this->mListener.userData);
    }
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        playerHandler.isSeeking = true;
    }
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    Float64 seconds = seekPos / 1000;
    [player seekToTime:CMTimeMakeWithSeconds(seconds, 1) completionHandler:^(BOOL finished) {
        if (this->mListener.SeekEnd) {
            this->mListener.SeekEnd(1, this->mListener.userData);
        }
        AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
        if (playerHandler) {
            playerHandler.isSeeking = false;
        }
    }];
}

int AppleAVPlayer::Stop()
{
    if (mStatus == PLAYER_STOPPED) {
        return 0;
    }
    mStatus = PLAYER_STOPPED;
    AVPlayer *avPlayer = (__bridge AVPlayer *) this->avPlayer;
    if (avPlayer) {
        [avPlayer pause];
    }
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (this->parentLayer != NULL) {
        [playerHandler removePlayerLayer];
    }
    //    CALayer *playerLayer = (CALayer *)CFBridgingRelease(this->parentLayer);
    if (this->playerHandler) {
        CFRelease(this->playerHandler);
    }
    this->playerHandler = nullptr;
    if (this->avPlayer) {
        CFRelease(this->avPlayer);
    }
    this->avPlayer = nullptr;
    if (this->sourceUrl) {
        CFRelease(this->sourceUrl);
    }
    this->sourceUrl = nullptr;
    if (this->resourceLoaderDelegate) {
        CFRelease(this->resourceLoaderDelegate);
    }
    this->resourceLoaderDelegate = nullptr;

    this->mListener = {
            nullptr,
    };
    this->mStreamInfos = nullptr;
    return 0;
}

PlayerStatus AppleAVPlayer::GetPlayerStatus() const
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    if (player.rate > 0) {
        return PLAYER_PLAYING;
    }
    return PLAYER_STOPPED;
}

int64_t AppleAVPlayer::GetDuration() const
{
    if (this->avPlayer == NULL) {
        return 0;
    }
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    if (isnan(CMTimeGetSeconds(player.currentItem.duration))) {
        return 0;
    }
    return (int64_t)(CMTimeGetSeconds(player.currentItem.duration) * 1000);
}

int64_t AppleAVPlayer::GetPlayingPosition()
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    NSTimeInterval currentTimeSeconds = CMTimeGetSeconds(player.currentTime);
    if (isnan(currentTimeSeconds)) {
        return 0;
    }
    return (int64_t)(currentTimeSeconds * 1000);
}

int64_t AppleAVPlayer::GetBufferPosition()
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    AVPlayerItem *playerItem = player.currentItem;
    int64_t position = [AppleAVPlayerUtil getBufferPosition:playerItem];
    return position;
}

void AppleAVPlayer::Mute(bool bMute)
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    if (bMute) {
        this->recordVolume = player.volume;
        player.volume = 0;
    } else {
        player.volume = this->recordVolume;
    }
}

bool AppleAVPlayer::IsMute() const
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    if ([player isMuted]) {
        return true;
    }
    return false;
}

void AppleAVPlayer::SetVolume(float volume)
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    player.volume = volume;
}
float AppleAVPlayer::GetVolume() const
{
    if (this->avPlayer == NULL) {
        return 1;
    }
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    return player.volume;
}
float AppleAVPlayer::GetVideoRenderFps()
{
    return 0;
}

void AppleAVPlayer::EnterBackGround(bool back)
{
    
}
void AppleAVPlayer::SetScaleMode(ScaleMode mode)
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    playerHandler.layerProcessor.scaleMode = mode;
}

ScaleMode AppleAVPlayer::GetScaleMode()
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    return playerHandler.layerProcessor.scaleMode;
}

void AppleAVPlayer::SetRotateMode(RotateMode mode)
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    playerHandler.layerProcessor.rotateMode = mode;
}

RotateMode AppleAVPlayer::GetRotateMode()
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    return playerHandler.layerProcessor.rotateMode;
}

void AppleAVPlayer::SetVideoBackgroundColor(uint32_t color)
{
    
}

void AppleAVPlayer::SetMirrorMode(MirrorMode mode)
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    playerHandler.layerProcessor.mirrorMode = mode;
}

MirrorMode AppleAVPlayer::GetMirrorMode()
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    return playerHandler.layerProcessor.mirrorMode;
}

int AppleAVPlayer::GetCurrentStreamIndex(StreamType type)
{
    return 0;
}
StreamInfo *AppleAVPlayer::GetCurrentStreamInfo(StreamType type)
{
    return nullptr;
}
int64_t AppleAVPlayer::GetMasterClockPts()
{
    return 0;
}
void AppleAVPlayer::SetTimeout(int timeout)
{
    
}
void AppleAVPlayer::SetDropBufferThreshold(int dropValue)
{
    
}
void AppleAVPlayer::SetDecoderType(DecoderType type)
{
    
}
DecoderType AppleAVPlayer::GetDecoderType()
{
    return DT_SOFTWARE;
}

void AppleAVPlayer::SetRefer(const char *refer)
{
    
}
void AppleAVPlayer::SetUserAgent(const char *userAgent)
{
    
}

void AppleAVPlayer::SetLooping(bool bCirclePlay)
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    playerHandler.isCirclePlay = bCirclePlay;
}

bool AppleAVPlayer::isLooping()
{
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    return playerHandler.isCirclePlay;
}

void AppleAVPlayer::CaptureScreen()
{
#if TARGET_OS_IPHONE
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *)this->playerHandler;
    UIImage *captureImage = [playerHandler captureScreen];
    NSData *imageData = UIImagePNGRepresentation(captureImage);
    CGSize imageSize = playerHandler.parentLayer.bounds.size;
    if (this->mListener.CaptureScreen) {
        this->mListener.CaptureScreen(imageSize.width, imageSize.height, imageData.bytes, this->mListener.userData);
    }
#endif
}

void AppleAVPlayer::GetVideoResolution(int &width, int &height)
{
    
}

void AppleAVPlayer::GetVideoRotation(int &rotation)
{
    
}

std::string AppleAVPlayer::GetPropertyString(PropertyKey key)
{
    return ICicadaPlayer::GetPropertyString(key);
}

int64_t AppleAVPlayer::GetPropertyInt(PropertyKey key)
{
    return ICicadaPlayer::GetPropertyInt(key);
}

float AppleAVPlayer::GetVideoDecodeFps()
{
    return 0;
}

int AppleAVPlayer::SetOption(const char *key, const char *value)
{
    NSString *kkey = [[NSString alloc] initWithUTF8String:key];
    if ([kkey isEqualToString:@"AVResourceLoaderDelegate"]) {
        NSString *addressStr = [[NSString alloc] initWithUTF8String:value];
        void *resourceLoaderDelegate = (void *)[addressStr integerValue];
        CFRetain(resourceLoaderDelegate);
        this->resourceLoaderDelegate = resourceLoaderDelegate;
        return 0;
    }
    return 0;
}

void AppleAVPlayer::GetOption(const char *key, char *value)
{
    
}

void AppleAVPlayer::setSpeed(float speed)
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    player.rate = speed;
}

float AppleAVPlayer::getSpeed()
{
    AVPlayer *player = (__bridge AVPlayer *)this->avPlayer;
    return player.rate;
}

void AppleAVPlayer::AddCustomHttpHeader(const char *httpHeader)
{
    
}

void AppleAVPlayer::RemoveAllCustomHttpHeader()
{
    
}

void AppleAVPlayer::addExtSubtitle(const char *uri)
{
    
}

int AppleAVPlayer::selectExtSubtitle(int index, bool bSelect)
{
    return 0;
}

int AppleAVPlayer::getCurrentStreamMeta(Stream_meta *meta, StreamType type)
{
    return 0;
}

void AppleAVPlayer::reLoad()
{
    
}

void AppleAVPlayer::SetAutoPlay(bool bAutoPlay)
{
    this->isAutoPlay = bAutoPlay;
}

bool AppleAVPlayer::IsAutoPlay()
{
    return this->isAutoPlay;
}

int AppleAVPlayer::invokeComponent(std::string content)
{
    return 0;
}


