//
//  AVPConfig.mm
//  AliPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018年 com.alibaba.AliyunPlayer. All rights reserved.
//

#import "AVDConfig.h"

@implementation AVDConfig

@synthesize timeoutMs;
@synthesize connnectTimoutMs;
@synthesize referer;
@synthesize userAgent;
@synthesize httpProxy;

- (instancetype)init{
    if (self = [super init]) {
        referer = nil;
        httpProxy = nil;
        userAgent = nil;
        connnectTimoutMs = 5000;
        timeoutMs = 15000;
    }
    return self;
}

@end

