//
// Created by moqi on 2020/7/20.
//

#ifdef __APPLE__
#include <TargetConditionals.h>
#include <render/audio/Apple/AFAudioSessionWrapper.h>
#endif

#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif
#include "AppleAVPlayer.h"
#include "AppleAVPlayerHandler.h"
#include "AppleAVPlayerLayerProcessor.h"
#include "AppleAVPlayerUtil.h"

using namespace Cicada;

AppleAVPlayer AppleAVPlayer::se(1);
AppleAVPlayer::AppleAVPlayer()
{}

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
    lock_guard<recursive_mutex> lock(mCreateMutex);
    if (this->playerHandler != NULL) {
        AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
        [playerHandler setmPlayerListener:Listener];
    }
    return 0;
}

void AppleAVPlayer::SetOnRenderCallBack(onRenderFrame cb, void *userData)
{}

void AppleAVPlayer::recheckHander()
{
    lock_guard<recursive_mutex> lock(mCreateMutex);
    if (this->playerHandler == NULL) {
        return;
    }
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (this->parentLayer != NULL) {
        playerHandler.parentLayer = (__bridge CALayer *) this->parentLayer;
    }
    if (this->avPlayer != NULL) {
        playerHandler.avplayer = (__bridge AVPlayer *) this->avPlayer;
    }
    [playerHandler setmPlayerListener:this->mListener];
}

void AppleAVPlayer::SetView(void *view)
{
    {
        lock_guard<recursive_mutex> lock(mCreateMutex);
        AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
        if (this->parentLayer) {
            if (playerHandler) {
                [playerHandler removePlayerLayer];
            }
            CFRelease(this->parentLayer);
        }
        this->parentLayer = view;
    }
    if (view) {
        CFRetain(view);
        this->recheckHander();
    }
}

void AppleAVPlayer::SetDataSource(const char *url)
{
    //    this->sourceUrl = (char *)url;
    NSString *urlString = [[NSString alloc] initWithUTF8String:url];
    {
        std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
        this->sourceUrl = (__bridge_retained void *) urlString;
    }
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
#if TARGET_OS_IPHONE
    AFAudioSessionWrapper::activeAudio();
#endif

    NSString *urlString = @"";
    {
        lock_guard<recursive_mutex> lock(mCreateMutex);
        if (this->sourceUrl) {
            urlString = (__bridge NSString *) this->sourceUrl;
        }
    }

    NSLog(@"Prepare url : %@", urlString);
    NSURL *mediaURL = [NSURL URLWithString:urlString];
    AVURLAsset *asset = [AVURLAsset assetWithURL:mediaURL];

    AVURLAsset *subtitleAsset = nil;
    if (this->subtitleUrl) {
        NSString *subtitleUrlString = (__bridge NSString *) this->subtitleUrl;
        NSURL *subtitleURL = [NSURL URLWithString:subtitleUrlString];
        subtitleAsset = [AVURLAsset assetWithURL:subtitleURL];
    }

    AVPlayerItem *item = nil;
    if (subtitleAsset && subtitleAsset.tracks.count > 0) {
        AVMutableComposition *mutableComposition = [[AVMutableComposition alloc] init];
        // origin tracks
        for (AVAssetTrack *track in asset.tracks) {
            AVMutableCompositionTrack *compositionTrack = [mutableComposition addMutableTrackWithMediaType:track.mediaType
                                                                                          preferredTrackID:track.trackID];
            [compositionTrack insertTimeRange:CMTimeRangeMake(kCMTimeZero, asset.duration) ofTrack:track atTime:kCMTimeZero error:nil];
        }
        // subtitle
        for (AVAssetTrack *track in subtitleAsset.tracks) {
            AVMutableCompositionTrack *compositionTrack = [mutableComposition addMutableTrackWithMediaType:track.mediaType
                                                                                          preferredTrackID:track.trackID];
            [compositionTrack insertTimeRange:CMTimeRangeMake(kCMTimeZero, asset.duration) ofTrack:track atTime:kCMTimeZero error:nil];
        }
        item = [AVPlayerItem playerItemWithAsset:mutableComposition];
    } else {
        {
            lock_guard<recursive_mutex> lock(mCreateMutex);
            if (this->resourceLoaderDelegate) {
                [asset.resourceLoader setDelegate:(__bridge id<AVAssetResourceLoaderDelegate>) this->resourceLoaderDelegate
                                            queue:dispatch_get_main_queue()];
            }
        }
        item = [AVPlayerItem playerItemWithAsset:asset];
    }

    UpdatePlayerStatus(PLAYER_PREPARING);

    AppleAVPlayerHandler *playerHandler = [[AppleAVPlayerHandler alloc] init];
    AVPlayer *player = [[AVPlayer alloc] initWithPlayerItem:item];
    {
        std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
        this->avPlayer = (__bridge_retained void *) player;
        this->playerHandler = (__bridge_retained void *) playerHandler;
    }
    this->recheckHander();

    // 初始化轨道信息
    NSArray<AVMediaCharacteristic> *array = [asset availableMediaCharacteristicsWithMediaSelectionOptions];
    NSMutableArray *selectionOptionArray = [NSMutableArray array];
    [array enumerateObjectsUsingBlock:^(AVMediaCharacteristic _Nonnull obj, NSUInteger idx, BOOL *_Nonnull stop) {
      AVMediaSelectionGroup *mediaGroup = [asset mediaSelectionGroupForMediaCharacteristic:obj];
      [mediaGroup.options enumerateObjectsUsingBlock:^(AVMediaSelectionOption *_Nonnull options, NSUInteger idx, BOOL *_Nonnull stop) {
        [selectionOptionArray addObject:options];
      }];
    }];
    playerHandler.selectionOptionArray = selectionOptionArray;

    int size = (int) selectionOptionArray.count;
    {
        std::lock_guard<std::recursive_mutex> lock(mCreateMutex);

        this->mStreamInfos = new StreamInfo *[size];
        [selectionOptionArray enumerateObjectsUsingBlock:^(AVMediaSelectionOption *_Nonnull options, NSUInteger idx, BOOL *_Nonnull stop) {
          auto *info = new StreamInfo();
          info->streamIndex = (int) idx;
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
    }

    if (this->mListener.StreamInfoGet) {
        this->mListener.StreamInfoGet((int64_t) size, this->mStreamInfos, this->mListener.userData);
    }

    if (this->isAutoPlay) {
        [player play];
        UpdatePlayerStatus(PLAYER_PLAYING);
    }
}

void AppleAVPlayer::Start()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player) {
        [player play];
        UpdatePlayerStatus(PLAYER_PLAYING);
    }
}

void AppleAVPlayer::Pause()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player) {
        [player pause];
        UpdatePlayerStatus(PLAYER_PAUSED);
    }
}

StreamType AppleAVPlayer::SwitchStream(int index)
{
    {
        std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
        AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
        AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
        if (playerHandler == nullptr || player == nullptr) {
            return ST_TYPE_UNKNOWN;
        }

        NSArray *optionArray = playerHandler.selectionOptionArray;
        AVMediaSelectionOption *option = optionArray[index];
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
    }

    StreamType type = ST_TYPE_UNKNOWN;
    {
        lock_guard<recursive_mutex> lock(mCreateMutex);

        if (mStreamInfos) {
            StreamInfo *info = this->mStreamInfos[index];
            type = info->type;
            if (this->mListener.StreamSwitchSuc) {
                this->mListener.StreamSwitchSuc(type, info, this->mListener.userData);
            }
        }
    }
    return type;
}

void AppleAVPlayer::SeekTo(int64_t seekPos, bool bAccurate)
{
    if (this->mListener.Seeking) {
        this->mListener.Seeking(1, this->mListener.userData);
    }

    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);

    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        playerHandler.isSeeking = true;
    }

    float rate = 0;
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player) {
        rate = player.rate;
        [player pause];
    }

    void (^completionHandler)(BOOL finished) = ^(BOOL finished) {
      player.rate = rate;
      if (this->mListener.SeekEnd) {
          this->mListener.SeekEnd(1, this->mListener.userData);
      }
      std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
      AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
      if (playerHandler) {
          playerHandler.isSeeking = false;
      }
    };

    /*
     * Work around seek to end, the avplayer can't perform seek to end
     */
    CMTime toleranceTime = kCMTimeZero;
    int64_t duration = GetDuration();
    if (seekPos + 1000 >= duration) {
        bAccurate = true;
        toleranceTime = CMTimeMakeWithSeconds(0.5, 1);
        seekPos = duration - 1000;
    }
    [player.currentItem cancelPendingSeeks];
    if (bAccurate) {
        [player seekToTime:CMTimeMakeWithSeconds((Float64) seekPos / 1000, 1)
                  toleranceBefore:toleranceTime
                   toleranceAfter:toleranceTime
                completionHandler:completionHandler];
    } else {
        [player seekToTime:CMTimeMakeWithSeconds((Float64) seekPos / 1000, 1) completionHandler:completionHandler];
    }
}

int AppleAVPlayer::Stop()
{
    if (mStatus == PLAYER_STOPPED) {
        return 0;
    }
    mStatus = PLAYER_STOPPED;

    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);

    AVPlayer *avPlayer = (__bridge AVPlayer *) this->avPlayer;
    if (avPlayer) {
        [avPlayer pause];
    }

    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler && this->parentLayer != NULL) {
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
    if (this->subtitleUrl) {
        CFRelease(this->subtitleUrl);
    }
    if (this->resourceLoaderDelegate) {
        CFRelease(this->resourceLoaderDelegate);
    }
    this->resourceLoaderDelegate = nullptr;

    //    this->mListener = {
    //            nullptr,
    //    };
    this->mStreamInfos = nullptr;
    return 0;
}

PlayerStatus AppleAVPlayer::GetPlayerStatus() const
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player && player.rate > 0) {
        return PLAYER_PLAYING;
    }
    return PLAYER_STOPPED;
}

int64_t AppleAVPlayer::GetDuration() const
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player == NULL) {
        return 0;
    }

    if (isnan(CMTimeGetSeconds(player.currentItem.duration))) {
        return 0;
    }
    return (int64_t)(CMTimeGetSeconds(player.currentItem.duration) * 1000);
}

int64_t AppleAVPlayer::GetPlayingPosition()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player == nullptr) {
        return 0;
    }

    NSTimeInterval currentTimeSeconds = CMTimeGetSeconds(player.currentTime);
    if (isnan(currentTimeSeconds)) {
        return 0;
    }
    return (int64_t)(currentTimeSeconds * 1000);
}

int64_t AppleAVPlayer::GetBufferPosition()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player == nullptr) {
        return 0;
    }

    AVPlayerItem *playerItem = player.currentItem;
    int64_t position = [AppleAVPlayerUtil getBufferPosition:playerItem];
    return position;
}

void AppleAVPlayer::Mute(bool bMute)
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player == nullptr) {
        return;
    }
    if (bMute) {
        this->recordVolume = player.volume;
        player.volume = 0;
    } else {
        player.volume = this->recordVolume;
    }
}

bool AppleAVPlayer::IsMute() const
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player && [player isMuted]) {
        return true;
    }
    return false;
}

void AppleAVPlayer::SetVolume(float volume)
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player) {
        player.volume = volume;
    }
}
float AppleAVPlayer::GetVolume() const
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player) {
        return player.volume;
    }
    return 1;
}
float AppleAVPlayer::GetVideoRenderFps()
{
    return 0;
}

void AppleAVPlayer::EnterBackGround(bool back)
{}
void AppleAVPlayer::SetScaleMode(ScaleMode mode)
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        playerHandler.layerProcessor.scaleMode = mode;
    }
}

ScaleMode AppleAVPlayer::GetScaleMode()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        return playerHandler.layerProcessor.scaleMode;
    } else {
        return SM_FIT;
    }
}

void AppleAVPlayer::SetRotateMode(RotateMode mode)
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        playerHandler.layerProcessor.rotateMode = mode;
    }
}

RotateMode AppleAVPlayer::GetRotateMode()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        return playerHandler.layerProcessor.rotateMode;
    } else {
        return ROTATE_MODE_0;
    }
}

void AppleAVPlayer::SetVideoBackgroundColor(uint32_t color)
{}

void AppleAVPlayer::SetMirrorMode(MirrorMode mode)
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        playerHandler.layerProcessor.mirrorMode = mode;
    }
}

MirrorMode AppleAVPlayer::GetMirrorMode()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        return playerHandler.layerProcessor.mirrorMode;
    } else {
        return MIRROR_MODE_NONE;
    }
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
{}
void AppleAVPlayer::SetDropBufferThreshold(int dropValue)
{}
void AppleAVPlayer::SetDecoderType(DecoderType type)
{}
DecoderType AppleAVPlayer::GetDecoderType()
{
    return DT_SOFTWARE;
}

void AppleAVPlayer::SetRefer(const char *refer)
{}
void AppleAVPlayer::SetUserAgent(const char *userAgent)
{}

void AppleAVPlayer::SetLooping(bool bCirclePlay)
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        playerHandler.isCirclePlay = bCirclePlay;
    }
}

bool AppleAVPlayer::isLooping()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        return playerHandler.isCirclePlay;
    } else {
        return false;
    }
}

void AppleAVPlayer::CaptureScreen()
{
#if TARGET_OS_IPHONE
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AppleAVPlayerHandler *playerHandler = (__bridge AppleAVPlayerHandler *) this->playerHandler;
    if (playerHandler) {
        UIImage *captureImage = [playerHandler captureScreen];
        NSData *imageData = UIImagePNGRepresentation(captureImage);
        CGSize imageSize = playerHandler.parentLayer.bounds.size;
        if (this->mListener.CaptureScreen) {
            this->mListener.CaptureScreen(imageSize.width, imageSize.height, imageData.bytes, this->mListener.userData);
        }
    }
#endif
}

void AppleAVPlayer::GetVideoResolution(int &width, int &height)
{}

void AppleAVPlayer::GetVideoRotation(int &rotation)
{}

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
        void *resourceLoaderDelegate = (void *) [addressStr integerValue];
        CFRetain(resourceLoaderDelegate);
        {
            lock_guard<recursive_mutex> lock(mCreateMutex);
            this->resourceLoaderDelegate = resourceLoaderDelegate;
        }
        return 0;
    }
    return 0;
}

void AppleAVPlayer::GetOption(const char *key, char *value)
{}

void AppleAVPlayer::setSpeed(float speed)
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player) {
        player.rate = speed;
    }
}

float AppleAVPlayer::getSpeed()
{
    std::lock_guard<std::recursive_mutex> lock(mCreateMutex);
    AVPlayer *player = (__bridge AVPlayer *) this->avPlayer;
    if (player) {
        return player.rate;
    } else {
        return 1;
    }
}

void AppleAVPlayer::AddCustomHttpHeader(const char *httpHeader)
{}

void AppleAVPlayer::RemoveAllCustomHttpHeader()
{}

void AppleAVPlayer::addExtSubtitle(const char *uri)
{
    NSString *urlString = [[NSString alloc] initWithUTF8String:uri];
    this->subtitleUrl = (__bridge_retained void *) urlString;
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
{}

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
int AppleAVPlayer::setStreamDelay(int index, int64_t time)
{
    return 0;
}
