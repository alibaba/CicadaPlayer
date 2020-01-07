//
//  ApsaraTestCase.h
//  AliPlayerAutoTestTests
//
//  Created by 郦立 on 2019/1/17.
//  Copyright © 2019年 wb-qxx397776. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CicadaPlayerSDK/CicadaPlayerSDK.h>

#define AliyunTestAysncTimeOut  30          //异步等待超时时间
#define WEAK_SELF  __weak typeof(self)weakSelf =self;

@interface ApsaraTestCase : NSObject

+ (NSArray <CicadaUrlSource *>*)urlSourceArray;

+ (NSArray *)uidSTSArray;

+ (NSArray *)uidURLArray;

+ (NSArray <NSUUID *>*)uuidArray;

+ (NSArray *)downloaderVidArray;

+ (NSString *)timeShiftUrlString;

+ (NSString *)timeShiftUrlCurrentString;

+ (NSString *)thumbnailVID;

+ (NSString *)outsideSubtitleURLString;

@end

















