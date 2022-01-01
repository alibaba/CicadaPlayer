//
//  CicadaConfig.mm
//  CicadaPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018年 com.alibaba.AliyunPlayer. All rights reserved.
//

#import "CicadaConfig.h"

@implementation CicadaConfig

@synthesize maxDelayTime;
@synthesize highBufferDuration;
@synthesize startBufferDuration;
@synthesize positionTimerIntervalMs;
@synthesize maxBufferDuration;
@synthesize networkTimeout;
@synthesize referer;
@synthesize userAgent;
@synthesize httpProxy;
@synthesize clearShowWhenStop;
@synthesize networkRetryCount;
@synthesize httpHeaders;
@synthesize liveStartIndex;
@synthesize disableAudio;
@synthesize disableVideo;

- (instancetype)init{
    if (self = [super init]) {
        referer = nil;
        httpProxy = nil;
        networkTimeout = 15000;
        maxBufferDuration = 50000;
        startBufferDuration = 500;
        highBufferDuration = 3000;
        maxDelayTime = 5000;
        clearShowWhenStop = NO;
        userAgent = nil;
        networkRetryCount = 2;
        httpHeaders = [[NSMutableArray alloc] init];
        liveStartIndex = -3;
        disableAudio = NO;
        disableVideo = NO;
        positionTimerIntervalMs = 500;
    }
    return self;
}

@end

