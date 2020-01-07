//
//  AVSettingAndConfigView.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AVBottomView.h"

@interface AVBottomView()

@property (strong) IBOutlet NSView *displayView;
@end

@implementation AVBottomView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        NSNib *xib = [[NSNib alloc] initWithNibNamed:@"AVSettingAndConfigView" bundle:nil];
        NSArray *viewsArray = [[NSArray alloc] init];
        self.controller = [[AVSettingAndConfigController alloc] init];
        [xib instantiateWithOwner:self.controller topLevelObjects:&viewsArray];
        for (int i = 0; i < viewsArray.count; i++) {
            if ([viewsArray[i] isKindOfClass:[NSView class]]) {
                self.displayView = viewsArray[i];
                break;
            }
        }

        [self.displayView setFrame:[self bounds]];
        NSLog(@"%@", self.displayView);
        [self addSubview:self.displayView];
    }
    return self;
}

- (void)awakeFromNib
{
    [self.controller setBoxDelegate];
}

- (void)layout {
    [super layout];
    self.displayView.frame = self.bounds;
}


@end

