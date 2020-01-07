//
//  AVSettingAndConfigView.h
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AVTopController.h"

@class AVTopView;

@protocol AVTopViewDelegate <NSObject>

@optional

/**
 进度条进度回调

 @param playerView playerView
 @param value 进度值
 */
- (void)topView:(AVTopView *)playerView progressSliderDidChange:(CGFloat)value;
/**
 全屏半屏调用

 @param playerView playerView
 @param full 状态值
 */
- (void)topView:(AVTopView *)playerView fullScreen:(BOOL)full;

@end


NS_ASSUME_NONNULL_BEGIN

@interface AVTopView : NSOpenGLView

@property (strong) IBOutlet AVTopController *topController;

@end

NS_ASSUME_NONNULL_END

