//
//  FMHud.m
//  FuDianBank
//
//  Created by 汪宁 on 2017/8/21.
//  Copyright © 2017年 WN. All rights reserved.
//

#import "HudTool.h"
#import "NSLabel.h"
#import "CicadaSourceChooserModel.h"
#import "MJExtension.h"
@interface HudTool()

@property (nonatomic, strong) NSView *shadowView;
@property (nonatomic, strong) NSView *loadingShadowView;
@property (nonatomic, strong) NSProgressIndicator* indicator;
@property (nonatomic, strong) NSAlert *alert;
@property (nonatomic, copy) NSString * checkUniqueId;
@property (nonatomic, copy) NSString * alertMessage;

@end
static HudTool * hudInstance = nil;

@implementation HudTool

+ (instancetype)sharedInstance {

    if (! hudInstance) {
        hudInstance = [[HudTool alloc]init];
    }

    return hudInstance;
}

+ (NSArray *)getSourceSamplesArray {
    NSString *path = [[NSBundle mainBundle] pathForResource:@"source" ofType:@"json"];
    unsigned long encode = CFStringConvertEncodingToNSStringEncoding(kCFStringEncodingUTF8);
    NSError *error;
    NSString *content = [NSString stringWithContentsOfFile:path encoding:encode error:&error];
    NSArray *array = [CicadaSourceChooserModel mj_objectArrayWithKeyValuesArray:content];
    return array;
}

+ (NSArray *)getDocumentMP4Array {
    NSString *documentPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSArray *fileList = [fileManager contentsOfDirectoryAtPath:documentPath error:nil];
    NSMutableArray *dirArray = [[NSMutableArray alloc] init];
    for (NSString *file in fileList) {
        if ([[file pathExtension] isEqualToString:@"mp4"] || [[file pathExtension] isEqualToString:@"mp3"] || [[file pathExtension] isEqualToString:@"flv"]) {
            NSString *filePath = [NSString stringWithFormat:@"%@/%@",documentPath,file];
            [dirArray addObject:filePath];
        }
    }
    return dirArray.copy;
}


+ (void)hudWithText:(NSString *)text {
    [[HudTool sharedInstance]showText:text];
}

+ (void)loadingHudToView:(NSView *)view {
    [[HudTool sharedInstance]loadingView:view];
}


+ (void)hideLoadingHudForView:(NSView *)view {
    [[HudTool sharedInstance]hideLoadingView:view];
}

+ (void)showAlert:(NSString *)text {
    [[HudTool sharedInstance]showAlertView:text];
}

- (void)showAlertView:(NSString *)text {
    
    if (!_alert) {

        _alert = [[NSAlert alloc]init];
        _alert.messageText = text;
        _alert.alertStyle = NSWarningAlertStyle;
        [_alert addButtonWithTitle:@"确定"];
        NSWindow *window =  [NSApplication sharedApplication].keyWindow;
        [_alert beginSheetModalForWindow:window completionHandler:^(NSModalResponse returnCode) {
            if (returnCode == NSAlertFirstButtonReturn ) {
                self->_alert = nil;
            }
            
        }];
    }
  
   
}

- (void)showText:(NSString *)text {
    CGSize size =[text boundingRectWithSize:CGSizeMake(280, 40) options:NSStringDrawingUsesFontLeading attributes:@{NSFontAttributeName:[NSFont systemFontOfSize:15]} context:nil].size;
    CGFloat width = size.width;
    if (size.width<80) {
        width = 80;
    }
    NSLabel *label = [[NSLabel alloc]initWithFrame:CGRectMake(0, 0, width+20, 40)];
    label.text =text;
    CGSize windowSize;
    windowSize =  [NSApplication sharedApplication].keyWindow.contentView.frame.size;


    label.frame = CGRectMake((windowSize.width -
                              width-20)/2 , 220, width+20, 40);

    [[NSApplication sharedApplication].keyWindow.contentView addSubview:label];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1.5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [label removeFromSuperview];

    });

}

+ (NSString *)infoStringDescribe:(CicadaMediaInfo *)info {
    if (!info) {
        return @"信息暂缺";
    }
    NSMutableString *backString = [NSMutableString string];

    if (info.tracks.count != 0) {
        [backString appendString:@"tracks:"];
        for (CicadaTrackInfo *trInfo in info.tracks) {
            [backString appendString:trInfo.description];
            [backString appendString:@","];
        }
        [backString deleteCharactersInRange:NSMakeRange(backString.length-1, 1)];
    }
    if (backString.length == 0) {
        return @"信息暂缺";
    }
    return backString.copy;
}

- (void)showText:(NSString *)text finishedBlock:(void (^)(void))finishedBlock{

    CGSize size =[text boundingRectWithSize:CGSizeMake(150, 500) options:NSStringDrawingUsesFontLeading attributes:@{NSFontAttributeName:[NSFont systemFontOfSize:15]} context:nil].size;
    CGFloat height = size.height;
    if (size.height<40) {
        height = 40;
    }
    NSTextField *label = [[NSTextField alloc]initWithFrame:CGRectMake(0, 0, 150, height)];
    //label.center = [UIApplication sharedApplication].keyWindow.center;
    label.stringValue =text;
    label.layer.masksToBounds = YES;
    label.layer.cornerRadius = 10;
    label.backgroundColor = (__bridge NSColor * _Nullable)([NSColor lightGrayColor].CGColor);
    label.font = [NSFont systemFontOfSize:15];
    label.textColor = [NSColor whiteColor];
    //label.textAlignment = NSTextAlignmentCenter;
    [[NSApplication sharedApplication].keyWindow.contentView addSubview:label];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [label removeFromSuperview];
        finishedBlock();
    });


}

- (void)loadingView:(NSView *)view {
    
    if (!_indicator) {
        _indicator = [[NSProgressIndicator alloc] initWithFrame:NSMakeRect((view.bounds.size.width - 30)/2, (view.bounds.size.height - 30)/2, 30, 30)];
        [_indicator setStyle:NSProgressIndicatorSpinningStyle];
        [_indicator startAnimation:nil];
        [view addSubview:_indicator];
        
    }
 
}

- (void)hideLoadingView:(NSView *)view{
    [_indicator stopAnimation:nil];
    [_indicator removeFromSuperview];
    
}


+ (NSTimeInterval)currentTimeInterval{
    
     return [[NSDate date] timeIntervalSince1970];
}



/*
 - (void)showWaitPanel{

 __block typeof(self) weakself = self;
 dispatch_async(dispatch_get_main_queue(), ^{
 if (!weakself.loadingShadowView) {
 NSWindow * window  = [NSApplication sharedApplication].keyWindow;
 weakself.loadingShadowView = [[NSView alloc]initWithFrame:[UIScreen mainScreen].bounds];
 weakself.loadingShadowView.backgroundColor = [NSColor clearColor].CGColor;
 [window.contentView addSubview:weakself.loadingShadowView];
 NSView * loadingView = [[NSView alloc]initWithFrame:CGRectMake(0, 0, 100, 100)];
 loadingView.layer.cornerRadius = 4;
 loadingView.layer.masksToBounds =YES;
 loadingView.center = weakself.loadingShadowView.center;//只能设置中心，不能设置大小
 loadingView.layer.backgroundColor = [NSColor grayColor].CGColor;
 [weakself.loadingShadowView addSubview:loadingView];
 NSImageView *imageView = [[NSImageView alloc]initWithFrame:CGRectMake(25, 15, 50, 75)];

 [loadingView addSubview:imageView];
 NSMutableArray *array = [[NSMutableArray alloc]init];

 NSImage *image = [[UIImage alloc]init];
 for (int i =0; i< 21; ++i) {
 if (i<10) {
 image = [UIImage imageNamed:[NSString stringWithFormat:@"toast_0000%d",i]];
 }else{
 image = [UIImage imageNamed:[NSString stringWithFormat:@"toast_000%d",i]];
 }
 [array addObject:image];
 }
 imageView.animationImages = array;
 imageView.animationDuration = 0.5;
 imageView.animationRepeatCount = 0;
 [imageView startAnimating];

 }
 });

 }
 */
- (void)hideAll {
    __block typeof(self) weakself = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (weakself.shadowView) {
            [weakself.shadowView removeFromSuperview];
            weakself.shadowView  = nil;
        }

        if (weakself.loadingShadowView) {
            [weakself.loadingShadowView removeFromSuperview];
            weakself.loadingShadowView  = nil;
        }

    });

}

- (void)hideWaitPanel {
    __block typeof(self) weakself = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        if (weakself.loadingShadowView) {
            [weakself.loadingShadowView removeFromSuperview];
            weakself.loadingShadowView  = nil;
        }
    });

}


@end

