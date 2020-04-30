//
//  LeftScrollView.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/2/15.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "LeftScrollView.h"

@interface LeftScrollView ()

@property (nonatomic, strong) NSView *view;

//右侧视频列表数据
@property (nonatomic, strong) NSArray *listArray;
@property (nonatomic, strong) NSArray *vidListArray;
@property (nonatomic, strong) NSArray *urlListArray;
@end

@implementation LeftScrollView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    // Drawing code here.
}

- (instancetype)initWithCoder:(NSCoder *)aDecoder {
    self = [super initWithCoder:aDecoder];
    if (self) {
        _listArray = [[NSArray alloc]init];
        _view = nil;
        NSNib *xib = [[NSNib alloc] initWithNibNamed:@"LeftScrollView" bundle:nil];
        NSArray *viewsArray = [[NSArray alloc] init];

        [xib instantiateWithOwner:self topLevelObjects:&viewsArray];
        for (int i = 0; i < viewsArray.count; i++) {
            if ([viewsArray[i] isKindOfClass:[NSView class]]) {
                _view = viewsArray[i];
                break;
            }
        }
        [self addSubview:_view];
        self.documentView = _view;
        self.hasVerticalScroller = YES;
        self.hasHorizontalScroller = NO;
        self.scrollerKnobStyle = NSScrollerKnobStyleDark;
    }
    return self;
}

- (IBAction)inputURL:(id)sender {
    [self.rightView loadViewWithViewType:inputURL];
}

- (IBAction)showLocalView:(id)sender {
    [self.rightView loadViewWithViewType:localURL];
}

- (IBAction)showUrlLinks:(id)sender {
    [self.rightView loadViewWithViewType:urlLinks];
}

- (IBAction)MultPlayer:(id)sender {
    [self.rightView loadViewWithViewType:MultPlayer];
}

- (void)layout {
    [super layout];
    self.view.frame = CGRectMake(0, 0, self.bounds.size.width, self.bounds.size.height+ 50);
}

@end

