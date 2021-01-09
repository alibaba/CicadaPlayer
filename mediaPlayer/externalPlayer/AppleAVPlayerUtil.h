//
//  AppleAVPlayerUtil.h
//  CicadaPlayerSDK
//
//  Created by zhou on 2020/7/26.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface AppleAVPlayerUtil : NSObject

+ (int64_t)getBufferPosition:(AVPlayerItem *)playerItem;

@end

NS_ASSUME_NONNULL_END
