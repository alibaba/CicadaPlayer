//
//  CicadaSourceChooserModel.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/2.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaSourceChooserModel.h"

@implementation CicadaSourceChooserModel

+ (NSDictionary *)mj_objectClassInArray{
    return @{@"samples" : @"CicadaSourceSamplesModel"};
}

- (NSString *)name {
    return NSLocalizedString(_name , nil);
}

@end


@implementation CicadaSourceSamplesModel

- (NSString *)name {
    return NSLocalizedString(_name , nil);
}

@end
