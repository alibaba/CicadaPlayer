//
//  ConfigSetView.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/2/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "ConfigSetView.h"

@implementation ConfigSetView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    if (@available(macOS 10.13, *)) {
        self.layer.backgroundColor = [NSColor colorNamed:@"setColor"].CGColor;
    } else {
        self.layer.backgroundColor = [NSColor whiteColor].CGColor;
    }
}

@end
