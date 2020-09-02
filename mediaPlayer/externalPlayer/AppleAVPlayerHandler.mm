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
            
        } else if (status == AVPlayerItemStatusReadyToPlay) {
            if (mPlayerListener.Prepared) {
                mPlayerListener.Prepared(mPlayerListener.userData);
            }
            if (mPlayerListener.FirstFrameShow) {
                mPlayerListener.FirstFrameShow(mPlayerListener.userData);
            }
            AVPlayerItem *item = (AVPlayerItem *)object;
            if (mPlayerListener.VideoSizeChanged) {
                mPlayerListener.VideoSizeChanged(item.presentationSize.width, item.presentationSize.height, mPlayerListener.userData);
            }
        } else if (status == AVPlayerItemStatusUnknown) {
            
        }
    } else if ([keyPath isEqualToString:@"loadedTimeRanges"]) {
        AVPlayerItem *playerItem = (AVPlayerItem *)object;
        int64_t position = [AppleAVPlayerUtil getBufferPosition:playerItem];
        if (mPlayerListener.BufferPositionUpdate) {
            mPlayerListener.BufferPositionUpdate(position, mPlayerListener.userData);
        }
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
    if (mPlayerListener.Completion) {
        mPlayerListener.Completion(mPlayerListener.userData);
    }
    if (self.isCirclePlay) {
        [self.avplayer seekToTime:CMTimeMakeWithSeconds(0, 1) completionHandler:^(BOOL finished) {
            [self.avplayer play];
            if (mPlayerListener.AutoPlayStart) {
                mPlayerListener.AutoPlayStart(mPlayerListener.userData);
            }
        }];
    }
}

- (void)setupPlayerLayer {
    if (self.playerLayer) {
        [self.playerLayer removeFromSuperlayer];
    }
    AVPlayerLayer *layer = [AVPlayerLayer playerLayerWithPlayer:self.avplayer];
    layer.frame = CGRectMake(0, 0, self.parentLayer.bounds.size.width, self.parentLayer.bounds.size.height);
    [self.parentLayer addSublayer:layer];
    self.playerLayer = layer;
    self.layerProcessor.playerLayer = layer;
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
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

@end
