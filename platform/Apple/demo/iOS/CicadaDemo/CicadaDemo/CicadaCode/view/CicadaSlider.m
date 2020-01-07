//
//  CicadaSlider.m
//  CicadaPlayerDemo
//
//  Created by ToT on 2019/10/24.
//  Copyright © 2019 com.alibaba. All rights reserved.
//

#import "CicadaSlider.h"

@interface CicadaSlider()

@property (nonatomic,assign)BOOL isAdsorb;

@end


@implementation CicadaSlider

- (CGRect)thumbRectForBounds:(CGRect)bounds trackRect:(CGRect)rect value:(float)value {
    if (self.isAdsorb) {
        if (value > 0.45 && value < 0.55) {
            return [super thumbRectForBounds:bounds trackRect:rect value:0.5];
        }else {
            self.isAdsorb = NO;
            return [super thumbRectForBounds:bounds trackRect:rect value:value];
        }
    }else {
        //因为不是每次都能准备回掉0.5，所以取值0.49-0.51
        if (value > 0.49 && value < 0.51) {
            self.isAdsorb = YES;
        }
        return [super thumbRectForBounds:bounds trackRect:rect value:value];
    }
}

@end
