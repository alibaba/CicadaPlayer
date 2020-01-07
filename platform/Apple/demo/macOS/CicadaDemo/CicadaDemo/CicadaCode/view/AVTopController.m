//
//  AVTopController.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/30.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AVTopController.h"

@interface AVTopController ()<NSWindowDelegate>

@property (nonatomic,strong)NSString *currentPositionString;
@property (nonatomic,strong)NSString *allPositionString;
@property (strong) IBOutlet NSButton *windowResizeButton;
@property (nonatomic, assign) BOOL isBegin;
@end

@implementation AVTopController


- (instancetype)init{
    if (self = [super init]) {
        _isBegin = NO;
        [[NSNotificationCenter defaultCenter]addObserver:self selector:@selector(windowResize) name:@"WindowResize" object:nil];
        [[NSNotificationCenter defaultCenter]addObserver:self selector:@selector(checkIsBegin) name:@"PlayerPrepare" object:nil];
        
    }
     return self;
}

- (void)windowResize{
    
    if ([_windowResizeButton.title isEqualToString:@"全屏"]) {
        [_windowResizeButton setTitle:@"半屏"];
    }else {
        [_windowResizeButton setTitle:@"全屏"];
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.

}
- (IBAction)fullScreen:(id)sender {
  _windowResizeButton = sender;
    NSLog(@"%@",self.view.window);
   
    [self.view.window toggleFullScreen:nil];
}

- (IBAction)valueChange:(id)sender {

    if (self.allPosition <= 0) {
        return;
    }
    _currentPosition = self.allPosition * self.progressSlider.floatValue/100;
    self.currentPositionString = [self getMMSSFromSS:[NSString stringWithFormat:@"%lld",self.currentPosition/1000]];
    [self setTimeLabelText];
    [self.player seekToTime:self.progressSlider.floatValue/100 *self.player.duration seekMode:CICADA_SEEKMODE_INACCURATE];
   
}


- (void)setBufferPosition:(int64_t)bufferPosition {
    _bufferPosition = bufferPosition;
    self.buffTextField.stringValue =  [NSString stringWithFormat:@"buffer: %@",[self getMMSSFromSS:[NSString stringWithFormat:@"%lld",bufferPosition/1000]]];
}

- (void)setCurrentPosition:(int64_t)currentPosition {
    if (self.allPosition != 0) {
        _currentPosition = currentPosition;
        self.currentPositionString = [self getMMSSFromSS:[NSString stringWithFormat:@"%lld",currentPosition/1000]];
        [self setTimeLabelText];
       CGFloat allPositionFloat = (CGFloat)self.allPosition;
        if (_isBegin == YES) {
             _isBegin = NO;
             self.progressSlider.floatValue = currentPosition/allPositionFloat * 100;
        }
        
//      if (self.progressSlider.floatValue - currentPosition/allPositionFloat * 100>0){
//            return;
//        }
        self.progressSlider.floatValue = currentPosition/allPositionFloat * 100;
    }

}
- (void)checkIsBegin {
    _isBegin = YES;
}

- (void)setAllPosition:(int64_t)allPosition {
    _allPosition = allPosition;
    self.allPositionString = [self getMMSSFromSS:[NSString stringWithFormat:@"%lld",allPosition/1000]];
    [self setTimeLabelText];
}

- (void)setTimeLabelText {
    if (!self.currentPositionString) {
        self.currentPositionString = @"00:00:00";
    }
    self.timeTextField.stringValue = [NSString stringWithFormat:@"%@/%@",self.currentPositionString,self.allPositionString];
}

- (NSString *)getMMSSFromSS:(NSString *)totalTime{
    NSInteger seconds = [totalTime integerValue];
    //format of hour
    NSString *str_hour = [NSString stringWithFormat:@"%02ld",seconds/3600];
    //format of minute
    NSString *str_minute = [NSString stringWithFormat:@"%02ld",(seconds%3600)/60];
    //format of second
    NSString *str_second = [NSString stringWithFormat:@"%02ld",seconds%60];
    //format of time
    NSString *format_time = [NSString stringWithFormat:@"%@:%@:%@",str_hour,str_minute,str_second];
    return format_time;
}




@end

