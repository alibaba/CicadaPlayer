//
//  SettingViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/4/4.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "SettingViewController.h"

@interface SettingViewController ()

@property (weak, nonatomic) IBOutlet UILabel *versionLabel;
@property (weak, nonatomic) IBOutlet UISwitch *hardwareSwitch;
@property (weak, nonatomic) IBOutlet UISegmentedControl *logLevel;

@end

@implementation SettingViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.versionLabel.text = [self.versionLabel.text stringByAppendingString:[CicadaPlayer getSDKVersion]];
    self.hardwareSwitch.on = [CicadaTool isHardware];
    [self logLevelChanged:self.logLevel];
}

- (IBAction)switchChange:(UISwitch *)sender {
    [CicadaTool saveIsHardware:sender.isOn];
}

- (IBAction)logLevelChanged:(id)sender {
    CicadaLogLevel level = CICADA_LOG_LEVEL_INFO;
    switch (self.logLevel.selectedSegmentIndex) {
        case 0:
            level = CICADA_LOG_LEVEL_ERROR;
            break;
        case 1:
            level = CICADA_LOG_LEVEL_INFO;
            break;
        case 2:
            level = CICADA_LOG_LEVEL_DEBUG;
            break;
        default:
            break;
    }

    [CicadaPlayer setLogCallbackInfo:level callbackBlock:nil];
}

@end









