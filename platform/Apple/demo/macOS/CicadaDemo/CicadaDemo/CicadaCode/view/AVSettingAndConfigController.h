//
//  AVSettingAndConfigController.h
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AVTopController.h"
NS_ASSUME_NONNULL_BEGIN

@interface AVSettingAndConfigController : NSViewController

/**
 xib View
 */
//@property (strong, nonatomic) NSView *view;

/**
 url播放源
 */
@property (strong) IBOutlet NSTextField *urlTextField;

@property (nonatomic,weak) CicadaPlayer *player;
@property (nonatomic,weak) AVTopController *topController;

@property (nonatomic, strong) NSArray * videoTracksArray;
@property (nonatomic, strong) NSArray * audioTracksArray;
@property (nonatomic, strong) NSArray * subtitleTracksArray;
@property (nonatomic, strong) NSArray * totalDataArray;
/**
 设置box的代理
 */
- (void)setBoxDelegate;

/**
 设置音量
 */
- (void)setVolume:(CGFloat)value;


@end

NS_ASSUME_NONNULL_END

