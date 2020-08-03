//
//  AppleAVPlayerLayerProcessor.m
//  CicadaPlayerSDK
//
//  Created by zhou on 2020/7/26.
//
#ifdef __APPLE__
#include <TargetConditionals.h>
#endif
#import "AppleAVPlayerLayerProcessor.h"
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif

@interface AppleAVPlayerLayerProcessor ()

@property (nonatomic, strong) AVPlayer *playerRecord;

@end

@implementation AppleAVPlayerLayerProcessor

- (instancetype)init {
    self = [super init];
#if TARGET_OS_IPHONE
    if (self) {
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillEnterForeground) name:UIApplicationWillEnterForegroundNotification object:nil];
    }
#endif
    return self;
}

- (void)setParentLayer:(CALayer *)parentLayer {
    _parentLayer = parentLayer;
    [parentLayer addObserver:self forKeyPath:@"bounds" options:NSKeyValueObservingOptionNew context:nil];
}

- (void)setScaleMode:(ScaleMode)scaleMode {
    _scaleMode = scaleMode;
}

- (void)setMirrorMode:(MirrorMode)mirrorMode {
    _mirrorMode = mirrorMode;
    if (mirrorMode == MIRROR_MODE_NONE) {
        self.playerLayer.transform = CATransform3DMakeRotation(0, 0, 0, 0);
    } else if (mirrorMode == MIRROR_MODE_HORIZONTAL) {
        self.playerLayer.transform = CATransform3DMakeRotation(M_PI, 0, 1, 0);
    } else if (mirrorMode == MIRROR_MODE_VERTICAL) {
        self.playerLayer.transform = CATransform3DMakeRotation(M_PI, 1, 0, 0);
    }
}

- (void)setRotateMode:(RotateMode)rotateMode {
    _rotateMode = rotateMode;
    if (rotateMode == ROTATE_MODE_0) {
        self.playerLayer.transform = CATransform3DMakeRotation(0, 0, 0, 0);
    } else if (rotateMode == ROTATE_MODE_90) {
        self.playerLayer.transform = CATransform3DMakeRotation(M_PI / 2, 0, 0, 1);
    } else if (rotateMode == ROTATE_MODE_180) {
        self.playerLayer.transform = CATransform3DMakeRotation(M_PI, 0, 0, 1);
    } else if (rotateMode == ROTATE_MODE_270) {
        self.playerLayer.transform = CATransform3DMakeRotation(M_PI / 2, 0, 0, -1);
    }
    CGRect bounds = self.parentLayer.bounds;
    self.playerLayer.frame = CGRectMake(0, 0, bounds.size.width, bounds.size.height);
}

- (void)dealloc {
    [self.parentLayer removeObserver:self forKeyPath:@"bounds"];
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context {
    if ([keyPath isEqualToString:@"bounds"]) {
        CGRect bounds = [change[NSKeyValueChangeNewKey] CGRectValue];
        self.playerLayer.frame = CGRectMake(0, 0, bounds.size.width, bounds.size.height);
    }
}

- (void)applicationDidEnterBackground {
    self.playerRecord = self.playerLayer.player;
    self.playerLayer.player = nil;
}

- (void)applicationWillEnterForeground {
    self.playerLayer.player = self.playerRecord;
}

@end
