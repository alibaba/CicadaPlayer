//
//  AppDelegate.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2018/12/28.
//  Copyright © 2018年 com.alibaba. All rights reserved.
//

#import "AppDelegate.h"

@interface AppDelegate ()

@end

@implementation AppDelegate

- (void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event {
    [self.window resignFirstResponder];
}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.

//#define SAVE_APP_LOG
#ifdef SAVE_APP_LOG
    NSArray *paths =NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES);
    NSString *documentDirectory = [paths objectAtIndex:0];
    NSString *logFilePath = [documentDirectory stringByAppendingPathComponent:@"app.log"];
    NSString *logFilePathOld = [documentDirectory stringByAppendingPathComponent:@"old.log"];
    //先删除已经存在的文件
    NSFileManager *defaultManager = [NSFileManager defaultManager];
    [defaultManager removeItemAtPath:logFilePathOld error:nil];
    [defaultManager moveItemAtPath:logFilePath toPath:logFilePathOld error:nil];

    // 将log输入到文件
    freopen([logFilePath cStringUsingEncoding:NSASCIIStringEncoding],"a+", stdout);
    freopen([logFilePath cStringUsingEncoding:NSASCIIStringEncoding],"a+", stderr);
#endif

    //不熄灭屏幕
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
    [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback error:nil];
    return YES;
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
    [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:nil];
}

//允许旋转屏幕
- (UIInterfaceOrientationMask)application:(UIApplication *)application supportedInterfaceOrientationsForWindow:(nullable UIWindow *)window {
    if (self.allowRotation == YES) {
        return UIInterfaceOrientationMaskAllButUpsideDown;
    }else{
        return UIInterfaceOrientationMaskPortrait;
    }
}

@end
