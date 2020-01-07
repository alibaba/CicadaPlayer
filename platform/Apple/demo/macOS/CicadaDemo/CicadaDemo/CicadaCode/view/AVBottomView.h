//
//  AVBottomView.h
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AVSettingAndConfigController.h"

NS_ASSUME_NONNULL_BEGIN

@interface AVBottomView : NSView

@property (strong) IBOutlet AVSettingAndConfigController *controller;

@end

NS_ASSUME_NONNULL_END

