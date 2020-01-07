//
//  NSLabel.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/31.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "NSLabel.h"
@interface NSLabel()

@property (nonatomic, strong)NSTextField *textField;

@end

@implementation NSLabel

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    // Drawing code here.
}

- (instancetype)initWithFrame:(NSRect)frameRect {
    if (self = [super initWithFrame:frameRect]){

        self.wantsLayer = YES;
        self.layer.masksToBounds = YES;
        self.layer.cornerRadius = 5;

        self.layer.backgroundColor = [NSColor colorWithRed:0.96 green:0.96 blue:0.96 alpha:1].CGColor;

        self.textField = [[NSTextField alloc]init];
        _textField.textColor = [NSColor blackColor];
        _textField.bordered = NO;
        _textField.editable = NO;
        _textField.backgroundColor = [NSColor clearColor];
        _textField.font = [NSFont systemFontOfSize:15];
        [self addSubview:_textField];

    }
    return self;
}

- (void)setText:(NSString *)text {
    _textField.stringValue = text;
}

- (void)layout {

    CGSize textSize =[_textField.stringValue boundingRectWithSize:CGSizeMake(280, 40) options:NSStringDrawingUsesFontLeading attributes:@{NSFontAttributeName:[NSFont systemFontOfSize:15]} context:nil].size;
    CGFloat width = textSize.width;
    CGSize size = self.frame.size;
    _textField.frame = CGRectMake((size.width - width-5)/2, 10, width+5, 20);

}

@end

