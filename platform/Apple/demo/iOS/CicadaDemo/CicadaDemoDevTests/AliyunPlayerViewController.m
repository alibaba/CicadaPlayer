//
//  AliyunPlayerViewController.m
//  CicadaPlayerAutoTest
//
//  Created by 郦立 on 2019/1/17.
//  Copyright © 2019年 wb-qxx397776. All rights reserved.
//

#import "AliyunPlayerViewController.h"

@interface AliyunPlayerViewController ()

@property (nonatomic, strong)dispatch_semaphore_t semaphore;


@end

@implementation AliyunPlayerViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
}

- (void)initPlayer {
    self.player = [[CicadaPlayer alloc] init];
    self.player.playerView = self.view;
    self.player.delegate = self;
    if (self.urlSource) {
        [self.player setUrlSource:self.urlSource];
    }
    if (self.urlSource) {
        [self.player prepare];
    }
}

- (void)initPlayerWithCachePath:(NSString *)path {
    self.player = [[CicadaPlayer alloc] init];
    self.player.playerView = self.view;
    self.player.delegate = self;
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    config.enable = YES;
    config.maxSizeMB = 200;
    config.maxDuration = 300;
    config.path = path;
    [self.player setCacheConfig:config];
    if (self.urlSource) {
        [self.player setUrlSource:self.urlSource];
    }
    if (self.urlSource) {
        [self.player prepare];
    }
}

- (void)destory {
    [self.player stop];
    [self.player destroy];
    self.player = nil;
    if (_semaphore) {  _semaphore = nil; }

}

- (void)pause {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.player pause];
        });
    });
}

- (void)redraw {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.player redraw];
        });
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            if (self.redrawCallback) {
                self.redrawCallback();
            }
        });
    });
}

- (void)reset {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.player reset];
        });
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            if (self.resetCallback) {
                self.resetCallback();
            }
        });
    });
}

- (void)stop {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.player stop];
        });
    });
}

- (void)seekToTime:(int64_t)time seekMode:(CicadaSeekMode)seekMode {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.player seekToTime:time seekMode:seekMode];
        });
    });
}

- (void)snapShot {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            [self.player snapShot];
        });
    });
}

- (void)selectTrack {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            CicadaTrackInfo *info = self.info.lastObject;
            [self.player selectTrack:info.trackIndex];
        });
    });
}

- (void)getMediaInfo {
    async_semaphore_wait(self.semaphore, ^{
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            if (self.getMediaInfoCallback) {
                CicadaMediaInfo *info = [self.player getMediaInfo];
                self.getMediaInfoCallback(info);
            }
        });
    });
}

#pragma mark delegate

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventType 播放器事件类型，@see CicadaEventType
 */
-(void)onPlayerEvent:(CicadaPlayer*)player eventType:(CicadaEventType)eventType {
    switch (eventType) {
        case CicadaEventPrepareDone: {
            [self.player start];
            if (_semaphore) {
                dispatch_semaphore_signal(self.semaphore);
            }
        }
            break;
        case CicadaEventFirstRenderedStart:
            break;
        case CicadaEventCompletion:
            break;
        case CicadaEventLoadingStart:
            break;
        case CicadaEventLoadingEnd:
            break;
        case CicadaEventSeekEnd:
            break;
        case CicadaEventLoopingStart:
            break;
        default:
            break;
    }
    if (self.onPlayerEventCallback) {
        self.onPlayerEventCallback(eventType);
    }
}

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventWithString 播放器事件类型
 @param description 播放器事件说明
 @see CicadaEventType
 */
-(void)onPlayerEvent:(CicadaPlayer*)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description {
    if (self.onPlayerEventWithStringCallback) {
        self.onPlayerEventWithStringCallback(eventWithString, description);
    }
}

/**
 @brief 错误代理回调
 @param player 播放器player指针
 @param errorModel 播放器错误描述，参考CicadaErrorModel
 */
- (void)onError:(CicadaPlayer*)player errorModel:(CicadaErrorModel *)errorModel {
    if (self.onErrorCallback) {
        self.onErrorCallback(errorModel);
    }
}

/**
 @brief 视频大小变化回调
 @param player 播放器player指针
 @param width 视频宽度
 @param height 视频高度
 @param rotation 视频旋转角度
 */
- (void)onVideoSizeChanged:(CicadaPlayer*)player width:(int)width height:(int)height rotation:(int)rotation {
    if (self.onVideoSizeChangedCallback) {
        self.onVideoSizeChangedCallback(width, height, rotation);
    }
}

/**
 @brief 视频当前播放位置回调
 @param player 播放器player指针
 @param position 视频当前播放位置
 */
- (void)onCurrentPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    if (self.onCurrentPositionUpdateCallback) {
        self.onCurrentPositionUpdateCallback(position);
    }
}

/**
 @brief 视频缓存位置回调
 @param player 播放器player指针
 @param position 视频当前缓存位置
 */
- (void)onBufferedPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    if (self.onBufferedPositionUpdateCallback) {
        self.onBufferedPositionUpdateCallback(position);
    }
}

/**
 @brief 视频缓冲进度回调
 @param player 播放器player指针
 @param progress 缓存进度0-100
 */
- (void)onLoadingProgress:(CicadaPlayer*)player progress:(float)progress {
    if (self.onLoadingProgressCallback) {
        self.onLoadingProgressCallback(progress);
    }
}

/**
 @brief 获取track信息回调
 @param player 播放器player指针
 @param info track流信息数组 参考AVPTrackInfo
 */
- (void)onTrackReady:(CicadaPlayer*)player info:(NSArray<CicadaTrackInfo*>*)info {
    self.info = info;
    if (self.onTrackReadyCallback) {
        self.onTrackReadyCallback(info);
    }
}

/**
 @brief track切换完成回调
 @param player 播放器player指针
 @param info 切换后的信息 参考AVPTrackInfo
 */
- (void)onTrackChanged:(CicadaPlayer*)player info:(CicadaTrackInfo*)info {
    if (self.onTrackChangedCallback) {
        self.onTrackChangedCallback(info);
    }
}

/**
 @brief 字幕显示回调
 @param player 播放器player指针
 @param index 字幕显示的索引号
 @param subtitle 字幕显示的字符串
 */
- (void)onSubtitleShow:(CicadaPlayer*)player index:(int)index subtitle:(NSString *)subtitle {
    if (self.onSubtitleShowCallback) {
        self.onSubtitleShowCallback(index, subtitle);
    }
}

/**
 @brief 字幕隐藏回调
 @param player 播放器player指针
 @param index 字幕显示的索引号
 */
- (void)onSubtitleHide:(CicadaPlayer*)player index:(int)index {
    if (self.onSubtitleHideCallback) {
        self.onSubtitleHideCallback(index);
    }
}

/**
 @brief 字幕显示回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 @param subtitle 字幕显示的字符串
 */
- (void)onSubtitleShow:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID subtitle:(NSString *)subtitle {
    if (self.onSubtitleShowCallback) {
        self.onSubtitleShowCallback(trackIndex, subtitle);
    }
}

/**
 @brief 字幕隐藏回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 */
- (void)onSubtitleHide:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID {
    if (self.onSubtitleHideCallback) {
        self.onSubtitleHideCallback(trackIndex);
    }
}

/**
 @brief 获取缩略图成功回调
 @param positionMs 指定的缩略图位置
 @param fromPos 此缩略图的开始位置
 @param toPos 此缩略图的结束位置
 @param image 缩图略图像指针,对于mac是NSImage，iOS平台是UIImage指针
 */
- (void)onGetThumbnailSuc:(int64_t)positionMs fromPos:(int64_t)fromPos toPos:(int64_t)toPos image:(id)image {
    if (self.onGetThumbnailSucCallback) {
        self.onGetThumbnailSucCallback(positionMs, fromPos, toPos, image);
    }
}

/**
 @brief 获取缩略图失败回调
 @param positionMs 指定的缩略图位置
 */
- (void)onGetThumbnailFailed:(int64_t)positionMs {
    if (self.onGetThumbnailFailedCallback) {
        self.onGetThumbnailFailedCallback(positionMs);
    }
}

/**
 @brief 播放器状态改变回调
 @param player 播放器player指针
 @param oldStatus 老的播放器状态 参考AVPStatus
 @param newStatus 新的播放器状态 参考AVPStatus
 */
- (void)onPlayerStatusChanged:(CicadaPlayer*)player oldStatus:(CicadaStatus)oldStatus newStatus:(CicadaStatus)newStatus {
    if (self.onPlayerStatusChangedCallback) {
        self.onPlayerStatusChangedCallback(oldStatus, newStatus);
    }
}

/**
 @brief 获取截图回调
 @param player 播放器player指针
 @param image 图像
 */
- (void)onCaptureScreen:(CicadaPlayer*)player image:(UIImage*)image {
    if (self.onCaptureScreenCallback) {
        self.onCaptureScreenCallback(image);
    }
}

/**
@brief SEI回调
@param type 类型
@param data 数据
@see AVPImage
*/
- (void)onSEIData:(CicadaPlayer *)player type:(int)type data:(NSString *)data {
    if (self.onSELDataCallBack) {
        self.onSELDataCallBack(data);
    }
}

- (void)onSubtitleExtAdded:(CicadaPlayer*)player trackIndex:(int)trackIndex URL:(NSString *)URL {
    if (self.onSubtitleExtAdded) {
        self.onSubtitleExtAdded(trackIndex, URL);
    }
}

#pragma mark private

//开启异步线程等待
void async_semaphore_wait(dispatch_semaphore_t semaphore,dispatch_block_t block) {
    dispatch_queue_t waitQueue = dispatch_queue_create("waitQueue", DISPATCH_QUEUE_SERIAL);
    dispatch_async(waitQueue, ^{
        dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
        if (block) {
            dispatch_async(dispatch_get_main_queue(), ^{
                block();
            });
        }
    });
}

- (dispatch_semaphore_t)semaphore {
    if (!_semaphore) {
        _semaphore = dispatch_semaphore_create(0);
    }
    return _semaphore;
}

@end






