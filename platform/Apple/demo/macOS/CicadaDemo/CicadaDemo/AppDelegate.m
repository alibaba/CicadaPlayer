//
//  AppDelegate.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AppDelegate.h"
@interface AppDelegate () <NSWindowDelegate>

@end

@implementation AppDelegate


- (void)setMainWindow:(NSWindowController *)mainWindow {
    _mainWindow = mainWindow;
    _mainWindow.window.delegate = self;
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    [[NSUserDefaults standardUserDefaults] setBool:YES forKey:@"NSConstraintBasedLayoutVisualizeMutuallyExclusiveConstraints"];

}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize {
    [[NSNotificationCenter defaultCenter]postNotificationName:@"WindowResize" object:nil];
   
    return frameSize;
    
}

- (NSRect)windowWillUseStandardFrame:(NSWindow*)window defaultFrame:(NSRect)newFrame {
    [[NSNotificationCenter defaultCenter]postNotificationName:@"WindowResize" object:nil];
    return newFrame;
    
}

- (BOOL)windowShouldClose:(NSWindow *)sender {
    
    [[NSNotificationCenter defaultCenter]postNotificationName:@"WindowClose" object:nil];
    return YES;
}


@end
