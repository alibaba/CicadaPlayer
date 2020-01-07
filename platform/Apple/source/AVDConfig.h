//
//  AVDConfig.h
//  AliPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018 com.alibaba.AliyunPlayer. All rights reserved.
//

#ifndef AVDConfig_h
#define AVDConfig_h

#import <Foundation/Foundation.h>

OBJC_EXPORT
@interface AVDConfig : NSObject

/**
 @brief 最大超时时间 默认15000毫秒
 */
/****
 @brief Maximum timeout time. Default: 15000 milliseconds.
 */
@property (nonatomic, assign) int timeoutMs;

/**
 @brief 最大连接超时时间 默认5000毫秒
 */
/****
 @brief Maximum connection timeout time. Default: 5000 milliseconds.
 */
@property (nonatomic, assign) int connnectTimoutMs;

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

@end

#endif /* AVDConfig_h */
