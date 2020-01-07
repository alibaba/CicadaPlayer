//
//  AVTopController.h
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/30.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN

@interface AVTopController : NSViewController

@property (nonatomic,weak)CicadaPlayer *player;
@property (strong) IBOutlet NSTextField *buffTextField;
@property (strong) IBOutlet NSTextField *timeTextField;
@property (strong) IBOutlet NSSlider *progressSlider;

/**
 缓存位置
 */
@property (nonatomic,assign)int64_t bufferPosition;

/**
 当前播放位置
 */
@property (nonatomic,assign)int64_t currentPosition;

/**
 视频总时长
 */
@property (nonatomic,assign)int64_t allPosition;

/**
 标题label
 */

@property (strong) IBOutlet NSTextField *subTitleTextField;

/**
 是否隐藏进度条
 */
@property (nonatomic,assign)BOOL hiddenSlider;



@end

NS_ASSUME_NONNULL_END

