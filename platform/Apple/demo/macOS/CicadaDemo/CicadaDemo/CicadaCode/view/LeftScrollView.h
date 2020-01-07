//
//  LeftScrollView.h
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/2/15.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "RightView.h"
NS_ASSUME_NONNULL_BEGIN

@interface LeftScrollView : NSScrollView

@property (nonatomic, strong) RightView *rightView;


@end

NS_ASSUME_NONNULL_END
