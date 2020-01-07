//
//  CicadaCacheConfig+refresh.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/6/5.
//  Copyright © 2019 com.alibaba. All rights reserved.
//

#import "CicadaCacheConfig+refresh.h"

@implementation CicadaCacheConfig (refresh)

- (instancetype)refreshConfigWithDictionary:(NSDictionary *)dictionary {
    if ([dictionary.allKeys containsObject:@"path"]) {
        self.path = dictionary[@"path"];
    }
    if ([dictionary.allKeys containsObject:@"maxDuration"]) {
        self.maxDuration = [dictionary[@"maxDuration"] longLongValue];
    }
    if ([dictionary.allKeys containsObject:@"maxSizeMB"]) {
        self.maxSizeMB = [dictionary[@"maxSizeMB"] intValue];
    }
    BOOL enable = [dictionary[@"enable"] boolValue];
    self.enable = enable;
    return self;
}

@end
