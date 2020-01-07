//
//  CicadaTool.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2018/12/29.
//  Copyright © 2018年 com.alibaba. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface CicadaTool : NSObject

/**
 返回SourceSamples数组

 @return SourceSamples数组
 */
+ (NSArray *)getSourceSamplesArray;

/**
 返回SourceURL数组
 
 @return SourceURL数组
 */
+ (NSArray *)getSourceURLArray;

/**
 返回CicadaConfig数组

 @return CicadaConfig数组
 */
+ (NSMutableArray *)getCicadaConfigArray;

/**
 返回Config字典
 
 @return Config字典
 */
+ (NSDictionary *)getConfigDictionary;

/**
 在指定view上显示hud

 @param text 文字
 @param view 目标view
 */
+ (void)hudWithText:(NSString *)text view:(UIView *)view;

/**
 在指定view上显示加载中

 @param view 目标view
 */
+ (void)loadingHudToView:(UIView *)view;

/**
 在指定view上隐藏加载中

 @param view 目标view
 */
+ (void)hideLoadingHudForView:(UIView *)view;

/**
 从int返回string

 @return string
 */
+ (NSString *)stringFromInt:(int)count;

/**
 返回Document目录所有mp4路径数组

 @return 所有mp4路径数组
 */
+ (NSArray *)getDocumentMP4Array;

/**
 返回media信息string

 @param info info
 @return 信息字符
 */
+ (NSString *)infoStringDescribe:(CicadaMediaInfo *)info;

/**
 返回当前时间戳

 @return 当前时间戳
 */
+ (NSTimeInterval)currentTimeInterval;

/**
 弹出默认alertView
 
 */
+ (void)showAlert:(NSString *)title sender:(UIViewController *)sender;

/**
 是否开启硬解

 @return 是否硬解
 */
+ (BOOL)isHardware;

/**
 保存是否硬解设置

 @param isHardware 是否硬解
 */
+ (void)saveIsHardware:(BOOL)isHardware;

/**
过滤HTML的string

@param html string
@return 过滤的string
*/
+ (NSString *)filterHTML:(NSString *)html;

@end






