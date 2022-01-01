//
//  AppleAVPlayerHandler.m
//  CicadaPlayerSDK
//
//  Created by zhou on 2020/7/26.
//
#import "AppleAVPlayerHandler.h"
#import "AppleAVPlayerUtil.h"
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif

@interface AppleAVPlayerHandler ()

@property (nonatomic, strong) CALayer *playerLayer;

@end

@implementation AppleAVPlayerHandler

- (instancetype) init {
    self = [super init];
    if (self) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playerTimeJumped:) name:AVPlayerItemTimeJumpedNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(playerDidPlayToEndTime:) name:AVPlayerItemDidPlayToEndTimeNotification object:nil];
        self.layerProcessor = [[AppleAVPlayerLayerProcessor alloc] init];
        self.isSeeking = false;
    }
    return self;
}

- (void)setmPlayerListener:(playerListener)playerListener {
    mPlayerListener = playerListener;
}

- (void)setParentLayer:(CALayer *)parentLayer {
    _parentLayer = parentLayer;
    self.layerProcessor.parentLayer = parentLayer;
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context{
    if ([keyPath isEqualToString:@"status"]) {
        //取出status的新值
        AVPlayerItemStatus status = (AVPlayerItemStatus)[change[NSKeyValueChangeNewKey] integerValue];
        if (status == AVPlayerItemStatusFailed) {
            auto item = (AVPlayerItem *) object;
            if (mPlayerListener.ErrorCallback) {
                NSString *errorDesc = item.error.localizedDescription;
                if (item.errorLog) {
                    errorDesc = [errorDesc stringByAppendingFormat:@"\n%@", item.errorLog.description];
                }
                mPlayerListener.ErrorCallback(item.error.code, [errorDesc UTF8String], mPlayerListener.userData);
            }
        } else if (status == AVPlayerItemStatusReadyToPlay) {
            if (mPlayerListener.Prepared) {
                mPlayerListener.Prepared(mPlayerListener.userData);
            }
            if (mPlayerListener.StatusChanged) {
                mPlayerListener.StatusChanged(PLAYER_PREPARING, PLAYER_PREPARED, mPlayerListener.userData);
            }
            if (mPlayerListener.FirstFrameShow) {
                mPlayerListener.FirstFrameShow(mPlayerListener.userData);
            }
            AVPlayerItem *item = (AVPlayerItem *)object;
            if (mPlayerListener.VideoSizeChanged) {
                mPlayerListener.VideoSizeChanged(static_cast<int64_t>(item.presentationSize.width),
                                                 static_cast<int64_t>(item.presentationSize.height), mPlayerListener.userData);
            }
            [self.layerProcessor setVideoSize:item.presentationSize];
        } else if (status == AVPlayerItemStatusUnknown) {
            
        }
    } else if ([keyPath isEqualToString:@"loadedTimeRanges"]) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        int64_t position = [AppleAVPlayerUtil getBufferPosition:playerItem];
        if (mPlayerListener.BufferPositionUpdate) {
            mPlayerListener.BufferPositionUpdate(position, mPlayerListener.userData);
        }
    } else if ([keyPath isEqualToString:@"playbackBufferEmpty"]) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        if (mPlayerListener.LoadingStart && playerItem.playbackBufferEmpty) {
            mPlayerListener.LoadingStart(mPlayerListener.userData);
        }
    } else if ([keyPath isEqualToString:@"playbackLikelyToKeepUp"]) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        if (mPlayerListener.LoadingEnd && playerItem.playbackLikelyToKeepUp) {
            mPlayerListener.LoadingEnd(mPlayerListener.userData);
        }
    } else if ([keyPath isEqualToString:@"playbackBufferFull"]) {

    }
}

- (void)setAvplayer:(AVPlayer *)avplayer {
    if (_avplayer) {
        [_avplayer removeTimeObserver:self.timeObserver];
        [_avplayer.currentItem removeObserver:self forKeyPath:@"status"];
        [_avplayer.currentItem removeObserver:self forKeyPath:@"loadedTimeRanges"];
    }
    _avplayer = avplayer;
    if (_avplayer) {
        __weak typeof(self) weakSelf = self;
        self.timeObserver = [_avplayer addPeriodicTimeObserverForInterval:CMTimeMake(1, 1) queue:dispatch_get_main_queue() usingBlock:^(CMTime time) {
            __strong typeof(self) strongSelf = weakSelf;
            if (!strongSelf.isSeeking) {
                playerListener playerListener = [strongSelf getListener];
                NSTimeInterval currentTime = CMTimeGetSeconds(time);
                int64_t position = (int64_t)(currentTime * 1000);
                if (playerListener.PositionUpdate) {
                    playerListener.PositionUpdate(position, playerListener.userData);
                }
            }
        }];
        [avplayer.currentItem addObserver:weakSelf forKeyPath:@"status" options:NSKeyValueObservingOptionNew context:nil];
        [avplayer.currentItem addObserver:weakSelf forKeyPath:@"loadedTimeRanges" options:NSKeyValueObservingOptionNew context:nil];
        [avplayer.currentItem addObserver:weakSelf forKeyPath:@"playbackBufferEmpty" options:NSKeyValueObservingOptionNew context:nil];
        [avplayer.currentItem addObserver:weakSelf forKeyPath:@"playbackLikelyToKeepUp" options:NSKeyValueObservingOptionNew context:nil];
        [avplayer.currentItem addObserver:weakSelf forKeyPath:@"playbackBufferFull" options:NSKeyValueObservingOptionNew context:nil];
        [self setupPlayerLayer];
    }
}

- (playerListener)getListener{
    return mPlayerListener;
}

- (void)playerTimeJumped:(NSNotification *)notification {

}

- (void)playerDidPlayToEndTime:(NSNotification *)notification {
    AVPlayerItem *item = notification.object;
    if (item != self.avplayer.currentItem) {
        return;
    }
    if (self.isCirclePlay) {
        [self.avplayer seekToTime:CMTimeMakeWithSeconds(0, 1) completionHandler:^(BOOL finished) {
            [self.avplayer play];
            if (mPlayerListener.AutoPlayStart) {
                mPlayerListener.AutoPlayStart(mPlayerListener.userData);
            }
            if (mPlayerListener.LoopingStart) {
                mPlayerListener.LoopingStart(mPlayerListener.userData);
            }
        }];
    } else if (mPlayerListener.Completion) {
        mPlayerListener.Completion(mPlayerListener.userData);
    }
}

- (void)setupPlayerLayer {
    if (self.playerLayer) {
        [self.playerLayer removeFromSuperlayer];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        AVPlayerLayer *layer = [AVPlayerLayer playerLayerWithPlayer:self.avplayer];
        layer.frame = CGRectMake(0, 0, self.parentLayer.bounds.size.width, self.parentLayer.bounds.size.height);
        [self.parentLayer addSublayer:layer];
        self.playerLayer = layer;
        self.layerProcessor.playerLayer = layer;
    });
}

- (void)removePlayerLayer
{
    if (self.playerLayer) {
        [self.playerLayer removeFromSuperlayer];
    }
    self.playerLayer = nullptr;
    self.layerProcessor.playerLayer = nullptr;
}
#if TARGET_OS_IPHONE
- (UIImage *)captureScreen{
    UIGraphicsBeginImageContext(self.parentLayer.bounds.size);
    [self.parentLayer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *captureImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    return captureImage;
}
#endif
- (void)dealloc {
    [self.avplayer removeTimeObserver:self.timeObserver];
    [self.avplayer.currentItem removeObserver:self forKeyPath:@"status"];
    [self.avplayer.currentItem removeObserver:self forKeyPath:@"loadedTimeRanges"];
    [self.avplayer.currentItem removeObserver:self forKeyPath:@"playbackBufferEmpty"];
    [self.avplayer.currentItem removeObserver:self forKeyPath:@"playbackLikelyToKeepUp"];
    [self.avplayer.currentItem removeObserver:self forKeyPath:@"playbackBufferFull"];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
