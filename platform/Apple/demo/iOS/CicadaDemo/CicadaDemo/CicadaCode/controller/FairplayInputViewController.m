//
//  FairplayInputViewController.m
//  CicadaDemo
//
//  Created by zhou on 2020/7/30.
//  Copyright © 2020 com.alibaba. All rights reserved.
//

#import "FairplayInputViewController.h"
#import "CicadaPlayerViewController.h"
#import "CicadaScanViewController.h"
#import "AssetLoaderDelegate.h"

@interface FairplayInputViewController ()

@property (nonatomic, strong) IBOutlet UITextField *mediaUrlTextField;
@property (nonatomic, strong) IBOutlet UITextField *certificateUrlTextField;
@property (nonatomic, strong) IBOutlet UITextField *licenseUrlTextField;
@property (nonatomic, assign) BOOL isClickedFlag;

@end

@implementation FairplayInputViewController

- (void)viewDidLoad {
    [super viewDidLoad];
}

/**
 进入URL播放
 @param sender 调用者
 */
- (IBAction)gotoURLPlay:(id)sender {
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        NSString *urlStr = self.mediaUrlTextField.text;
        CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:urlStr];
        CicadaPlayerViewController *vc = [[CicadaPlayerViewController alloc]init];
        vc.urlSource = source;
        NSString *certificateUrl = self.certificateUrlTextField.text;
        NSString *licenseUrl = self.licenseUrlTextField.text;
        AssetLoaderDelegate *delegate = [[AssetLoaderDelegate alloc] initWithCertificateUrl:certificateUrl licenseUrl:licenseUrl];
        vc.useFairPlay = YES;
        vc.avResourceLoaderDelegate = delegate;
        [self.navigationController pushViewController:vc animated:YES];
    }
}

- (IBAction)pushToScan:(id)sender {
    if (self.isClickedFlag == NO) {
        self.isClickedFlag = YES;
        CicadaScanViewController *vc = [[CicadaScanViewController alloc]init];
        vc.scanTextCallBack = ^(NSString *text) {
            self.mediaUrlTextField.text = text;
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
