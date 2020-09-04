//
//  AppleAVPlayerLayerProcessor.h
//  CicadaPlayerSDK
//
//  Created by zhou on 2020/7/26.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "native_cicada_player_def.h"

NS_ASSUME_NONNULL_BEGIN

@interface AppleAVPlayerLayerProcessor : NSObject

@property (nonatomic, strong) CALayer *parentLayer;
@property (nonatomic, strong) AVPlayerLayer *playerLayer;

@property (nonatomic, assign) ScaleMode scaleMode;
@property (nonatomic, assign) MirrorMode mirrorMode;
@property (nonatomic, assign) RotateMode rotateMode;

- (void)setVideoSize:(CGSize)videoSize;

@end

NS_ASSUME_NONNULL_END
