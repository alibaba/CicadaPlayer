//
//  AliPlayerViewController.m
//  AliPlayerDemo
//
//  Created by 汪宁 on 2019/1/28.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "AliPlayerViewController.h"
#import "AVTopView.h"
#import "AVBottomView.h"
#import "AVPShowImageView.h"
#import <IOKit/pwr_mgt/IOPMLib.h>
@interface AliPlayerViewController ()<AVTopViewDelegate, CicadaDelegate>
/**
 配置视图
 */
@property (strong) IBOutlet AVBottomView *bottomView;

/**
 播放视图
 */
@property (strong) IBOutlet AVTopView *topView;

/**
 播放器
 */
@property (strong) IBOutlet NSOpenGLView *playerView;
@property (nonatomic,strong)CicadaPlayer *player;


@end

@implementation AliPlayerViewController

- (void)preventSystemSleep {
    
    CFStringRef reasonForActivity= CFSTR("Describe Activity Type");
    
    IOPMAssertionID assertionID;
    IOReturn success = IOPMAssertionCreateWithName(kIOPMAssertionTypeNoDisplaySleep,
                                                   kIOPMAssertionLevelOn, reasonForActivity, &assertionID);
    if (success == kIOReturnSuccess)
    {
        
    }
}

- (void)allowSleep {
    
    CFStringRef reasonForActivity= CFSTR("Describe Activity Type");
    
    IOPMAssertionID assertionID;
    IOReturn success = IOPMAssertionCreateWithName(nil,
                                                   kIOPMAssertionLevelOn, reasonForActivity, &assertionID);
    if (success == kIOReturnSuccess)
    {
        
    }
}

- (instancetype)initWithCoder:(NSCoder *)coder {
    if (self = [super initWithCoder:coder]) {
        [[NSNotificationCenter defaultCenter]addObserver:self selector:@selector(windowClose) name:@"WindowClose" object:nil];
    }
    return self;
}

- (void)windowClose {
    
    if (self.player) {
        [self.player stop];
        [self.player destroy];
    }
    
}

-(void)viewDidDisappear {
    [super viewDidDisappear];
    [self.player pause];
    [self allowSleep];
}


- (void)viewWillAppear {
    [super viewWillAppear];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    [self preventSystemSleep];
    self.view.window.collectionBehavior = NSWindowCollectionBehaviorFullScreenPrimary;
    self.player = [[CicadaPlayer alloc] init];
    self.player.autoPlay = YES;
    self.playerView.wantsLayer = YES;
    self.playerView.layer.backgroundColor = [NSColor blackColor].CGColor;
    self.player.playerView = self.playerView;
    self.player.delegate = self;
    self.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFIT;
    //[self.settingAndConfigView setVolume:self.player.volume];
    self.player.delegate = self;
    self.bottomView.controller.player = self.player;
    self.topView.topController.player = self.player;
    self.bottomView.controller.topController = self.topView.topController;
    
}

#pragma mark AVTopViewDelegate

- (void)topView:(AVTopView *)playerView progressSliderDidChange:(CGFloat)value {
    NSLog(@"%f",value);
    [self.player seekToTime:value*self.player.duration seekMode:CICADA_SEEKMODE_INACCURATE];
}


#pragma mark CicadaDelegate

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventType 播放器事件类型，@see CicadaEventType
 */
-(void)onPlayerEvent:(CicadaPlayer*)player eventType:(CicadaEventType)eventType{
    switch (eventType) {
        case CicadaEventPrepareDone: {
            if (self.player.duration == 0) {
                self.topView.topController.hiddenSlider = YES;
            }else {
                self.topView.topController.allPosition = self.player.duration;
            }

            [[HudTool sharedInstance]showText:@"准备完成"];
        }
            break;
        case CicadaEventFirstRenderedStart:
             [[HudTool sharedInstance]showText:@"首帧显示"];
            break;
        case CicadaEventCompletion:
             [[HudTool sharedInstance]showText:@"播放完成"];
            break;
        case CicadaEventLoadingStart:
             [[HudTool sharedInstance]showText:@"缓冲开始"];
            break;
        case CicadaEventLoadingEnd:
             [[HudTool sharedInstance]showText:@"缓冲完成"];
            break;
        case CicadaEventSeekEnd:
             [[HudTool sharedInstance]showText:@"跳转完成"];
            break;
        case CicadaEventLoopingStart:
             [[HudTool sharedInstance]showText:@"循环播放开始"];
            break;
        default:
            break;
    }
}

/**
 @brief 错误代理回调
 @param player 播放器player指针
 @param errorModel 播放器错误描述，参考AliVcPlayerErrorModel
 */
- (void)onError:(CicadaPlayer*)player errorModel:(CicadaErrorModel *)errorModel {
   
    NSString * errorMessage = [NSString stringWithFormat:@"code:%ld error:%@",errorModel.code,errorModel.message];
    [HudTool showAlert:errorMessage];
    
    [self.player stop];
    self.topView.topController.bufferPosition = 0;
    self.topView.topController.currentPosition = 0;
    self.topView.topController.allPosition = 0;
    self.bottomView.controller.totalDataArray =@[];
    
}

/**
 @brief 视频大小变化回调
 @param player 播放器player指针
 @param width 视频宽度
 @param height 视频高度
 @param rotation 视频旋转角度
 */
- (void)onVideoSizeChanged:(CicadaPlayer*)player width:(int)width height:(int)height rotation:(int)rotation {

}

/**
 @brief 视频当前播放位置回调
 @param player 播放器player指针
 @param position 视频当前播放位置
 */
- (void)onCurrentPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    self.topView.topController.currentPosition = position;
}

/**
 @brief 视频缓存位置回调
 @param player 播放器player指针
 @param position 视频当前缓存位置
 */
- (void)onBufferedPositionUpdate:(CicadaPlayer*)player position:(int64_t)position{
    self.topView.topController.bufferPosition = position;
}

/**
 @brief 视频缓冲进度回调
 @param player 播放器player指针
 @param progress 缓存进度0-100
 */
- (void)onLoadingProgress:(CicadaPlayer*)player progress:(float)progress {

}

/**
 @brief 获取track信息回调
 @param player 播放器player指针
 @param info track流信息数组 参考CicadaTrackInfo
 */
- (void)onTrackReady:(CicadaPlayer*)player info:(NSArray<CicadaTrackInfo*>*)info {

    NSMutableArray * videoTracksArray = [NSMutableArray array];
    NSMutableArray * audioTracksArray = [NSMutableArray array];
    NSMutableArray * subtitleTracksArray = [NSMutableArray array];

    for (int i=0; i<info.count; i++) {
        CicadaTrackInfo* track = [info objectAtIndex:i];
        switch (track.trackType) {
            case CICADA_TRACK_VIDEO: {
                [videoTracksArray addObject:track];
            }
                break;
            case CICADA_TRACK_AUDIO: {
                [audioTracksArray addObject:track];
            }
                break;
            case CICADA_TRACK_SUBTITLE: {
                [subtitleTracksArray addObject:track];
            }
                break;
            default:
                break;
        }
    }

    if (videoTracksArray.count > 0) {
        CicadaTrackInfo *autoInfo = [[CicadaTrackInfo alloc]init];
        autoInfo.trackIndex = -1;
        autoInfo.description = @"AUTO";
        [videoTracksArray insertObject:autoInfo atIndex:0];
    }

    self.bottomView.controller.videoTracksArray = videoTracksArray;
    self.bottomView.controller.audioTracksArray = audioTracksArray;
    self.bottomView.controller.subtitleTracksArray = subtitleTracksArray;
    self.bottomView.controller.totalDataArray = @[videoTracksArray,audioTracksArray,subtitleTracksArray];
}

/**
 @brief track切换完成回调
 @param player 播放器player指针
 @param info 切换后的信息 参考CicadaTrackInfo
 */
- (void)onTrackChanged:(CicadaPlayer*)player info:(CicadaTrackInfo*)info {
    NSString *str = [[NSString alloc] initWithFormat:@"%d", info.trackBitrate];
    NSString *hudText = [str stringByAppendingString:@"切换成功"];
    [[HudTool sharedInstance]showText:hudText];
}

/**
 @brief 字幕显示回调
 @param player 播放器player指针
 @param index 字幕显示的索引号
 @param subtitle 字幕显示的字符串
 */
- (void)onSubtitleShow:(CicadaPlayer*)player index:(int)index subtitle:(NSString *)subtitle {
    self.topView.topController.subTitleTextField.hidden = NO;
    self.topView.topController.subTitleTextField.stringValue = subtitle;
}

/**
 @brief 字幕隐藏回调
 @param player 播放器player指针
 @param index 字幕显示的索引号
 */
- (void)onSubtitleHide:(CicadaPlayer*)player index:(int)index {
    self.topView.topController.subTitleTextField.hidden = YES;
}

/**
 @brief 获取缩略图成功回调
 @param positionMs 指定的缩略图位置
 @param fromPos 此缩略图的开始位置
 @param toPos 此缩略图的结束位置
 @param image 缩图略图像指针,对于mac是NSImage，iOS平台是UIImage指针
 */
- (void)onGetThumbnailSuc:(int64_t)positionMs fromPos:(int64_t)fromPos toPos:(int64_t)toPos image:(id)image {

}

/**
 @brief 获取缩略图失败回调
 @param positionMs 指定的缩略图位置
 */
- (void)onGetThumbnailFailed:(int64_t)positionMs {

}

/**
 @brief 播放器状态改变回调
 @param player 播放器player指针
 @param oldStatus 老的播放器状态 参考CicadaStatus
 @param newStatus 新的播放器状态 参考CicadaStatus
 */
- (void)onPlayerStatusChanged:(CicadaPlayer*)player oldStatus:(CicadaStatus)oldStatus newStatus:(CicadaStatus)newStatus {

}

/**
 @brief 获取截图回调
 @param player 播放器player指针
 @param image 图像
 */
- (void)onCaptureScreen:(CicadaPlayer*)player image:(CicadaImage*)image {
    NSLog(@"onCaptureScreen:%@", image);
    [AVPShowImageView showWithImage:image inView:self.view];
}

- (void)setUrlSource:(CicadaUrlSource *)urlSource {
    [self.player setUrlSource:urlSource];
}

@end

