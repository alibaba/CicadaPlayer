//
//  AppleAVPlayerHandler.h
//  CicadaPlayerSDK
//
//  Created by zhou on 2020/7/26.
//

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

- (instancetype)init;
- (void)setmPlayerListener:(playerListener)playerListener;
- (UIImage *)captureScreen;

@end


NS_ASSUME_NONNULL_END
