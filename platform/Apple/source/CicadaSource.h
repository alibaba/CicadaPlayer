//
//  CicadaSource.h
//  CicadaPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018 com.alibaba.AliyunPlayer. All rights reserved.
//

#ifndef CicadaSource_h
#define CicadaSource_h

#import <Foundation/Foundation.h>

OBJC_EXPORT
@interface CicadaSource : NSObject

/**
 @brief 封面地址
 */
/****
 @brief The URL of the album cover.
 */
@property (nonatomic, copy) NSString* coverURL;

/**
 @brief 视频标题
 */
/****
 @brief The title of the video.
 */
@property (nonatomic, copy) NSString* title;

/**
 @brief 期望播放的清晰度
 */
/****
 @brief The preferred definition for playback.
 */
@property (nonatomic, copy) NSString* quality;

/**
 @brief 是否强制使用此清晰度。如果强制，则在没有对应清晰度的情况下播放不了。
 */
/****
 @brief Whether to force the player to play the media with the specified definition. If the media does not support the specified definition, then it cannot be played.
 */
@property (nonatomic, assign) BOOL forceQuality;

@end


OBJC_EXPORT
@interface CicadaUrlSource : CicadaSource

/**
 @brief init CicadaUrlSource with url
 @param url video url address
 */
- (instancetype) urlWithString:(NSString*)url;

/**
 @brief init CicadaUrlSource with filepath
 @param url file path
 */
- (instancetype) fileURLWithPath:(NSString*)url;

/**
 @brief player url
 */
@property (nonatomic, copy) NSURL* playerUrl;

/**
 @brief cache file path, app may cache the beginning of remote mp4 file to local path,
 * set it to player to improve loading speed.
 */
@property (nonatomic, copy) NSString* cacheFile;

@end

#endif /* CicadaSource_h */
