//
//  CicadaSource.mm
//  CicadaPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018年 com.alibaba.AliyunPlayer. All rights reserved.
//

#import "CicadaSource.h"

@interface CicadaSource ()
@end

@implementation CicadaSource

@synthesize title;
@synthesize coverURL;
@synthesize quality;

@end

@interface CicadaUrlSource ()
@end

@implementation CicadaUrlSource
@synthesize playerUrl;

- (instancetype)init{
    if (self = [super init]) {
        self.playerUrl = nil;
    }
    return self;
}

-(instancetype)urlWithString:(NSString *)url
{
    url = [url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    self.playerUrl = [NSURL URLWithString:url];
    
    return self;
}

-(instancetype)fileURLWithPath:(NSString *)url
{
    url = [url stringByAddingPercentEscapesUsingEncoding:NSUTF8StringEncoding];
    self.playerUrl = [NSURL fileURLWithPath:url];

    return self;
} 

@end

