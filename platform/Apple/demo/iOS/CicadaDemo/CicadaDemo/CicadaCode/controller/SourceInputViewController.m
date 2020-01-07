//
//  SourceInputViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2018/12/29.
//  Copyright © 2018年 com.alibaba. All rights reserved.
//

#import "SourceInputViewController.h"
#import "CicadaPlayerViewController.h"
#import "CicadaScanViewController.h"

@interface SourceInputViewController ()<UITextFieldDelegate>

@property (weak, nonatomic) IBOutlet UITextField *URLTextField;

@end

@implementation SourceInputViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    //开发测试用
    self.URLTextField.text = @"http://player.alicdn.com/video/aliyunmedia.mp4";
}

/**
 进入URL播放

 @param sender 调用者
 */
- (IBAction)gotoURLPlay:(id)sender {
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        NSString *urlStr = self.URLTextField.text;
        CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:urlStr];
        CicadaPlayerViewController *vc = [[CicadaPlayerViewController alloc]init];
        vc.urlSource = source;
        [self.navigationController pushViewController:vc animated:YES];
    }
}

- (IBAction)pushToScan:(id)sender {
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        CicadaScanViewController *vc = [[CicadaScanViewController alloc]init];
        vc.scanTextCallBack = ^(NSString *text) {
            self.URLTextField.text = text;
        };
        [self.navigationController pushViewController:vc animated:YES];
    }
}

#pragma mark textDelegate

- (BOOL)textFieldShouldReturn:(UITextField *)textField {
    [textField resignFirstResponder];
    return YES;
}

@end








