//
//  AppleAVPlayerHandler.h
//  CicadaPlayerSDK
//
//  Created by zhou on 2020/7/26.
//

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#import "native_cicada_player_def.h"

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "AppleAVPlayerLayerProcessor.h"

NS_ASSUME_NONNULL_BEGIN

@interface AppleAVPlayerHandler : NSObject {
    playerListener mPlayerListener;
}

@property (nonatomic, strong) CALayer *parentLayer;
@property (nonatomic, strong) AVPlayer *avplayer;
@property (nonatomic, strong) NSArray *selectionOptionArray;
@property (nonatomic, strong) id timeObserver;
@property (nonatomic, assign) BOOL isCirclePlay;
@property (nonatomic, strong) AppleAVPlayerLayerProcessor *layerProcessor;
@property(nonatomic, assign) BOOL isSeeking;

- (instancetype)init;
- (void)setmPlayerListener:(playerListener)playerListener;
- (void)removePlayerLayer;
#if TARGET_OS_IPHONE
- (UIImage *)captureScreen;
#endif
@end


NS_ASSUME_NONNULL_END
