//
//  CicadaConfig.h
//  CicadaPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018 com.alibaba.AliyunPlayer. All rights reserved.
//

#ifndef CicadaConfig_h
#define CicadaConfig_h

#import <Foundation/Foundation.h>

OBJC_EXPORT
@interface CicadaConfig : NSObject

/**
 @brief 直播最大延迟 默认5000毫秒，单位毫秒
 */
/****
 @brief The maximum broadcasting delay. Default: 5000 milliseconds, Unit: millisecond.
 */
@property (nonatomic, assign) int maxDelayTime;

/**
 @brief 卡顿后缓存数据的高水位，当播放器缓存数据大于此值时开始播放，单位毫秒
 */
/****
 @brief The size of data to be cached before the player can be resumed from playback lag. When the size of the data cached by the player reaches this value, the playback is resumed, Unit: millisecond.
 */
@property (nonatomic, assign) int highBufferDuration;

/**
 @brief 开始起播缓存区数据长度，默认500ms，单位毫秒
 */
/****
 @brief The size of the cache data required for starting playback. Default: 500 milliseconds, Unit: millisecond.
 */
@property (nonatomic, assign) int startBufferDuration;

/**
 @brief 播放器最大的缓存数据长度，默认50秒，单位毫秒
 */
/****
 @brief The maximum size of cache data. Default: 50 seconds, Unit: millisecond.
 */
@property (nonatomic, assign) int maxBufferDuration;

/**
 @brief 网络超时时间，默认15秒，单位毫秒
 */
/****
 @brief Network connection timeout time. Default: 15 seconds, Unit: millisecond.
 */
@property (nonatomic, assign) int networkTimeout;

/**
 @brief 网络重试次数，每次间隔networkTimeout，networkRetryCount=0则表示不重试，重试策略app决定，默认值为2
 */
/****
 @brief The maximum network reconnection attempts. Default: 2. networkTimeout specifies the reconnection interval. networkRetryCount=0 indicates that automatic network reconnection is disabled. The reconnection policy varies depending on the app.
 */
@property (nonatomic, assign) int networkRetryCount;

/**
 @brief 请求referer
 */
/****
 @brief Request Referer.
 */
@property (nonatomic, copy) NSString *referer;

/**
 @brief user Agent
 */
/****
 @brief UserAgent.
 */
@property (nonatomic, copy) NSString *userAgent;

/**
 @brief httpProxy代理
 */
/****
 @brief HTTP proxy.
 */
@property (nonatomic, copy) NSString *httpProxy;

/**
 @brief 调用stop停止后是否显示最后一帧图像，YES代表清除显示，黑屏，默认为NO
 */
/****
 @brief Whether to clear the last frame when the player is stopped. Set to YES to clear the last frame and a black view is displayed. Default: NO.
 */
@property (nonatomic, assign) BOOL clearShowWhenStop;

/**
 @brief 添加自定义header
 */
/****
 @brief Add a custom header.
 */
@property (nonatomic, copy) NSMutableArray *httpHeaders;

@end

#endif /* CicadaConfig_h */
