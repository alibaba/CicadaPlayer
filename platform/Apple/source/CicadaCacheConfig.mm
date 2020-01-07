//
//  CicadaCacheConfig.mm
//  CicadaPlayerSDK
//
//  Created by huang_jiafa on 2019/05/31.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#import "CicadaCacheConfig.h"

@implementation CicadaCacheConfig

@synthesize path;
@synthesize maxDuration;
@synthesize maxSizeMB;
@synthesize enable;

- (instancetype)init{
    if (self = [super init]) {
        path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
        path = [[NSString alloc] initWithFormat:@"%@/",path];
//        path = NSTemporaryDirectory();
        maxDuration = 0;
        maxSizeMB = 0;
        enable = NO;
    }

    return self;
}

@end
