//
//  AVSettingAndConfigView.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AVTopView.h"

@interface AVTopView ()

@property (strong) IBOutlet NSView *displayView;
@end

@implementation AVTopView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        NSNib *xib = [[NSNib alloc] initWithNibNamed:@"AVTopView" bundle:nil];
        NSArray *viewsArray = [[NSArray alloc] init];
        self.topController = [[AVTopController alloc] init];
        [xib instantiateWithOwner:self.topController topLevelObjects:&viewsArray];
        for (int i = 0; i < viewsArray.count; i++) {
            if ([viewsArray[i] isKindOfClass:[NSView class]]) {
                self.displayView = viewsArray[i];
                break;
            }
        }
        [self.displayView setFrame:[self bounds]];
        [self addSubview:self.displayView];
    }
    return self;
}

- (void)awakeFromNib {
    self.displayView.wantsLayer = YES;
}

- (void)layout {
    [super layout];
    self.displayView.frame = self.bounds;
}

@end

