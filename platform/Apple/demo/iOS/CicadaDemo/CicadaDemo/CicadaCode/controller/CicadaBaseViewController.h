//
//  CicadaBaseViewController.h
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/2.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface CicadaBaseViewController : UIViewController

/**
 防止重复点击 ,如果为YES,则不让重复点击
 */
@property(nonatomic,assign)BOOL isClickedFlag;

@end
