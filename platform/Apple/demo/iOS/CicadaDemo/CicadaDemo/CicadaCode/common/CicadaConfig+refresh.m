//
//  CicadaConfig+refresh.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/8.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaConfig+refresh.h"

@implementation CicadaConfig (refresh)

- (instancetype)refreshConfigWithArray:(NSArray *)array {
    if (array.count != 11) {
        return self;
    }
        
    int startBufferDuration = [array[0] intValue];
    int highBufferDuration = [array[1] intValue];
    int maxBufferDuration = [array[2] intValue];
    int maxDelayTime = [array[3] intValue];
    int networkTimeout = [array[4] intValue];
    int maxProbeSize = [array[5] intValue];
    NSString *referer = array[6];
    NSString *httpProxy = array[7];
    NSString *userAgent = array[8];
    int retryCount = [array[9] intValue];
    NSString *clearShowWhenStop = array[10];
    
    self.startBufferDuration = startBufferDuration;
    self.highBufferDuration = highBufferDuration;
    self.maxBufferDuration = maxBufferDuration;
    self.maxDelayTime = maxDelayTime;
    self.networkTimeout = networkTimeout;
    self.maxProbeSize = maxProbeSize;
    self.networkRetryCount = retryCount;
    if (referer.length != 0) {
        self.referer = referer;
    }
    if (httpProxy.length != 0) {
        self.httpProxy = httpProxy;
    }
    if (userAgent.length != 0) {
        self.userAgent = userAgent;
    }
    if ([clearShowWhenStop isEqualToString:@"1"]) {
        self.clearShowWhenStop = YES;
    }else {
        self.clearShowWhenStop = NO;
    }
    return self;
}

@end








