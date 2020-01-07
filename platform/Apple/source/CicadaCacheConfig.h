//
//  CicadaCacheConfig.h
//  CicadaPlayerSDK
//
//  Created by huang_jiafa on 2019/05/31.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#ifndef CicadaCacheConfig_h
#define CicadaCacheConfig_h

#import <Foundation/Foundation.h>

OBJC_EXPORT
@interface CicadaCacheConfig : NSObject

/**
 @brief 缓存目录
 */
/****
 @brief The cache directory.
 */
@property (nonatomic, copy) NSString *path;

/**
 @brief 单个视频缓存的最大时长，单位秒，即某个视频的时长超过maxDuration将不会被缓存
 */
/****
 @brief The maximum length of a single video that can be cached. Unit: seconds. Videos that exceed the maximum length are not cached.
 */
@property (nonatomic, assign) long maxDuration;

/**
 @brief 所有缓存最大占用空间，单位：MB
 */
/****
 @brief The maximum cache memory size. Unit: MB.
 */
@property (nonatomic, assign) int maxSizeMB;

/**
 @brief 是否开启缓存。默认关闭。
 */
/****
 @brief Enable or disable content caching. Default: disabled.
 */
@property (nonatomic, assign) BOOL enable;

@end

#endif /* CicadaCacheConfig_h */
