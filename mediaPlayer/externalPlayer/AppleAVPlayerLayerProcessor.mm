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

@property (nonatomic) CGSize videoSize;
@property CATransform3D scaleTransform;
@property CATransform3D mirrorTransform;
@property CATransform3D rotateTransform;

@property (nonatomic,assign) BOOL isFillWidth;

@end


@implementation AppleAVPlayerLayerProcessor

- (instancetype)init {
    self = [super init];
    if (self) {
#if TARGET_OS_IPHONE
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidEnterBackground) name:UIApplicationDidEnterBackgroundNotification object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationWillEnterForeground) name:UIApplicationWillEnterForegroundNotification object:nil];
#endif
        _scaleTransform = CATransform3DMakeRotation(0, 0, 0, 0);
        _mirrorTransform = CATransform3DMakeRotation(0, 0, 0, 0);
        _rotateTransform = CATransform3DMakeRotation(0, 0, 0, 0);
    }
    return self;
}

- (void)setParentLayer:(CALayer *)parentLayer {
    _parentLayer = parentLayer;
    _parentLayer.masksToBounds = YES;
    [parentLayer addObserver:self forKeyPath:@"bounds" options:NSKeyValueObservingOptionNew context:nil];
}

- (void)setScaleMode:(ScaleMode)scaleMode {
    _scaleMode = scaleMode;
    [self applayTransform];
}

- (void)setMirrorMode:(MirrorMode)mirrorMode {
    _mirrorMode = mirrorMode;
    if (mirrorMode == MIRROR_MODE_NONE) {
        _mirrorTransform = CATransform3DMakeRotation(0, 0, 0, 0);
    } else if (mirrorMode == MIRROR_MODE_HORIZONTAL) {
        _mirrorTransform = CATransform3DMakeRotation(M_PI, 0, 1, 0);
    } else if (mirrorMode == MIRROR_MODE_VERTICAL) {
        _mirrorTransform = CATransform3DMakeRotation(M_PI, 1, 0, 0);
    }
    
    [self applayTransform];
}

- (void)setRotateMode:(RotateMode)rotateMode {
    _rotateMode = rotateMode;
    if (rotateMode == ROTATE_MODE_0) {
        _rotateTransform = CATransform3DMakeRotation(0, 0, 0, 0);
    } else if (rotateMode == ROTATE_MODE_90) {
        _rotateTransform = CATransform3DMakeRotation(M_PI / 2, 0, 0, 1);
    } else if (rotateMode == ROTATE_MODE_180) {
        _rotateTransform = CATransform3DMakeRotation(M_PI, 0, 0, 1);
    } else if (rotateMode == ROTATE_MODE_270) {
        _rotateTransform = CATransform3DMakeRotation(M_PI / 2, 0, 0, -1);
    }
    
    [self applayTransform];
}

-(void)applayTransform{
    CATransform3D transform = CATransform3DConcat(_mirrorTransform,_rotateTransform);
    
    
    CGRect bounds = self.playerLayer.bounds;
    if (_scaleMode == SM_FIT) {
        float scale = 1;
        if(_isFillWidth){
            scale = self.playerLayer.bounds.size.width/[self getVideoSize].width;
        }else{
            scale = self.playerLayer.bounds.size.height/[self getVideoSize].height;
        }
        _scaleTransform = CATransform3DMakeScale(scale, scale, 1);
    }else if (_scaleMode == SM_CROP){
        float scale = 1;
        if(!_isFillWidth){
            scale = self.playerLayer.bounds.size.width/[self getVideoSize].width;
        }else{
            scale = self.playerLayer.bounds.size.height/[self getVideoSize].height;
        }
        _scaleTransform = CATransform3DMakeScale(scale, scale, 1);
    }else if (_scaleMode == SM_EXTRACTTOFIT){
        float scalex;
        float scaley;
        if(!_isFillWidth){
            scalex = self.playerLayer.bounds.size.width/[self getVideoSize].width;
            scaley = self.playerLayer.bounds.size.height/[self getVideoSize].height;
        }else{
            scalex = self.playerLayer.bounds.size.width/[self getVideoSize].width;
            scaley = self.playerLayer.bounds.size.height/[self getVideoSize].height;
        }
        _scaleTransform = CATransform3DMakeScale(scalex, scaley, 1);
    }
    
    self.playerLayer.transform = CATransform3DConcat(transform,_scaleTransform);
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

- (void)setVideoSize:(CGSize)videoSize{
    float scale = 1;
    _isFillWidth = self.playerLayer.bounds.size.width/self.playerLayer.bounds.size.height < videoSize.width/videoSize.height;
    if(_isFillWidth){
        scale = self.playerLayer.bounds.size.width/videoSize.width;
    }else{
        scale = self.playerLayer.bounds.size.height/videoSize.height;
    }
    _videoSize = CGSizeMake(videoSize.width*scale, videoSize.height*scale);
}

- (CGSize)getVideoSize{
    if (_rotateMode%180) {
        return CGSizeMake(_videoSize.height,_videoSize.width);
    }
    return _videoSize;
}

@end
