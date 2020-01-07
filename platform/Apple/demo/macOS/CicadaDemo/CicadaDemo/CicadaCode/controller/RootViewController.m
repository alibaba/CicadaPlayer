//
//  RootViewController.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/2/15.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "RootViewController.h"
#import "RightView.h"
#import "LeftScrollView.h"
@interface RootViewController ()

@property (strong) IBOutlet RightView *rightView;
@property (strong) IBOutlet LeftScrollView *leftView;

@end

@implementation RootViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    self.leftView.rightView = self.rightView;
    self.rightView.vc = self;
    
}

@end
