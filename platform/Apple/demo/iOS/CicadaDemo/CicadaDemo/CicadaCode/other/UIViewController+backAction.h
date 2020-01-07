//
//  UIViewController+backAction.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/10.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <UIKit/UIKit.h>

@protocol BackActionProtocol <NSObject>

@optional

-(BOOL)navigationShouldPopOnBackButton;

@end

@interface UIViewController (backAction)<BackActionProtocol>

@end
