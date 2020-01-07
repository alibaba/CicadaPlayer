//
//  AVPBaseViewController.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AVPBaseViewController.h"

@interface AVPBaseViewController ()

@end

@implementation AVPBaseViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)viewWillAppear{

    [super viewWillAppear];
    
    self.view.window.titlebarAppearsTransparent = YES;

//    self.view.window.styleMask = self.view.window.styleMask | NSWindowStyleMaskFullSizeContentView;
//    self.view.window.titleVisibility = NSWindowTitleHidden;
    [self.view.window setContentSize:CGSizeMake(900, 500)];
    [self.view.window setContentMinSize:CGSizeMake(900, 500)];
    
}

@end

