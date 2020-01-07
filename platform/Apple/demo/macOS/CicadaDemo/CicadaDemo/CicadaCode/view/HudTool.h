//
//  FMHud.h
//  FuDianBank
//
//  Created by 汪宁 on 2017/8/21.
//  Copyright © 2017年 WN. All rights reserved.
//

#import <Cocoa/Cocoa.h>

typedef void(^FMhandle)(void);

@interface HudTool : NSObject

+ (instancetype)sharedInstance;

/**
 在指定view上显示加载中
 
 @param view 目标view
 */
+ (void)loadingHudToView:(NSView *)view;

/**
 在指定view上隐藏加载中
 
 @param view 目标view
 */
+ (void)hideLoadingHudForView:(NSView *)view;

// 加载文件source.json 数据

+ (NSArray *)getSourceSamplesArray;
// 本地播放视频地址
+ (NSArray *)getDocumentMP4Array;
// toast
+ (void)hudWithText:(NSString *)text;
+ (void)showAlert:(NSString *)text;
- (void)showText:(NSString *)text;
- (void)showText:(NSString *)text finishedBlock:(void (^)(void))finishedBlock;

//
+ (NSString *)infoStringDescribe:(CicadaMediaInfo *)info;

+ (NSTimeInterval)currentTimeInterval;

@end

