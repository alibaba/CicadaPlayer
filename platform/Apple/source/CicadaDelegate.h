//
//  CicadaDelegate.h
//  CicadaPlayerSDK
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018 com.alibaba.AliyunPlayer. All rights reserved.
//

#ifndef CicadaDelegate_h
#define CicadaDelegate_h

@class CicadaPlayer;
@class CicadaTrackInfo;

#import <Foundation/Foundation.h>
#import "CicadaDef.h"

@protocol CicadaDelegate <NSObject>
@optional

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventType 播放器事件类型
 @see CicadaEventType
 */
/****
 @brief Player event callback.
 @param player Player pointer.
 @param eventType Player event type.
 @see CicadaEventType
 */
-(void)onPlayerEvent:(CicadaPlayer*)player eventType:(CicadaEventType)eventType;

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventWithString 播放器事件类型
 @param description 播放器事件说明
 @see CicadaEventType
 */
/****
 @brief Player event callback.
 @param player Player pointer.
 @param eventWithString Player event type.
 @param description Player event description.
 @see CicadaEventType
 */
-(void)onPlayerEvent:(CicadaPlayer*)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description;

/**
 @brief 错误代理回调
 @param player 播放器player指针
 @param errorModel 播放器错误描述，参考CicadaErrorModel
 @see CicadaErrorModel
 */
/****
 @brief Proxy error callback.
 @param player Player pointer.
 @param errorModel Player error description. See CicadaErrorModel.
 @see CicadaErrorModel
 */
- (void)onError:(CicadaPlayer*)player errorModel:(CicadaErrorModel *)errorModel;

/**
 @brief 视频大小变化回调
 @param player 播放器player指针
 @param width 视频宽度
 @param height 视频高度
 @param rotation 视频旋转角度
 */
/****
 @brief Video size change callback.
 @param player Player pointer.
 @param width Video width.
 @param height Video height.
 @param rotation Video rotate angle.
 */
- (void)onVideoSizeChanged:(CicadaPlayer*)player width:(int)width height:(int)height rotation:(int)rotation;

/**
 @brief 视频当前播放位置回调
 @param player 播放器player指针
 @param position 视频当前播放位置
 */
/****
 @brief Current playback position callback.
 @param player Player pointer.
 @param position Current playback position.
 */
- (void)onCurrentPositionUpdate:(CicadaPlayer*)player position:(int64_t)position;

/**
 @brief 视频缓存位置回调
 @param player 播放器player指针
 @param position 视频当前缓存位置
 */
/****
 @brief Current cache position callback.
 @param player Player pointer.
 @param position Current cache position.
 */
- (void)onBufferedPositionUpdate:(CicadaPlayer*)player position:(int64_t)position;

/**
 @brief 视频缓冲进度回调
 @param player 播放器player指针
 @param progress 缓存进度0-100
 */
/****
 @brief Buffer progress callback.
 @param player Player pointer.
 @param progress Buffer progress: from 0 to 100.
 */
- (void)onLoadingProgress:(CicadaPlayer*)player progress:(float)progress;

/**
 @brief 获取track信息回调
 @param player 播放器player指针
 @param info track流信息数组
 @see CicadaTrackInfo
 */
/****
 @brief Track information callback.
 @param player Player pointer.
 @param info An array of track information.
 @see CicadaTrackInfo
 */
- (void)onTrackReady:(CicadaPlayer*)player info:(NSArray<CicadaTrackInfo*>*)info;

/**
 @brief track切换完成回调
 @param player 播放器player指针
 @param info 切换后的信息 参考CicadaTrackInfo
 @see CicadaTrackInfo
 */
/****
 @brief Track switchover completion callback.
 @param player Player pointer.
 @param info Track switchover completion information. See CicadaTrackInfo.
 @see CicadaTrackInfo
 */
- (void)onTrackChanged:(CicadaPlayer*)player info:(CicadaTrackInfo*)info;

/**
 @brief 外挂字幕被添加
 @param player 播放器player指针
 @param index 字幕显示的索引号
 @param URL 字幕url
 */
/****
 @brief External subtitles have been added
 @param player Player pointer.
 @param trackIndex Subtitle index.
 @param URL subtitle url
 */
- (void)onSubtitleExtAdded:(CicadaPlayer*)player trackIndex:(int)trackIndex URL:(NSString *)URL;

/**
 @brief 字幕显示回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 @param subtitle 字幕显示的字符串
 */
/****
 @brief Show subtitle callback.
 @param player Player pointer.
 @param trackIndex Subtitle stream index.
 @param subtitleID Subtitle ID.
 @param subtitle Subtitle string.
 */
- (void)onSubtitleShow:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID subtitle:(NSString *)subtitle;

/**
 @brief 字幕隐藏回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 */
/****
 @brief Hide subtitle callback.
 @param player Player pointer.
 @param trackIndex Subtitle stream index.
 @param subtitleID Subtitle ID.
 */
- (void)onSubtitleHide:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID;

/**
 @brief 获取缩略图成功回调
 @param positionMs 指定的缩略图位置
 @param fromPos 此缩略图的开始位置
 @param toPos 此缩略图的结束位置
 @param image 缩图略图像指针,对于mac是NSImage，iOS平台是UIImage指针
 */
/****
 @brief Thumbnail image retrieval success callback.
 @param positionMs The specified thumbnail image position.
 @param fromPos The initial position of the thumbnail image.
 @param toPos The end position of the thumbnail image.
 @param image Thumbnail image pointer. Mac: NSImage. iOS: UIImage.
 */
- (void)onGetThumbnailSuc:(int64_t)positionMs fromPos:(int64_t)fromPos toPos:(int64_t)toPos image:(id)image;

/**
 @brief 获取缩略图失败回调
 @param positionMs 指定的缩略图位置
 */
/****
 @brief Thumbnail image retrieval failure callback.
 @param positionMs The specified thumbnail image position.
 */
- (void)onGetThumbnailFailed:(int64_t)positionMs;

/**
 @brief 播放器状态改变回调
 @param player 播放器player指针
 @param oldStatus 老的播放器状态 参考CicadaStatus
 @param newStatus 新的播放器状态 参考CicadaStatus
 @see CicadaStatus
 */
/****
 @brief Player status update callback.
 @param player Player pointer.
 @param oldStatus The previous status. See CicadaStatus.
 @param newStatus The updated status. See CicadaStatus.
 @see CicadaStatus
 */
- (void)onPlayerStatusChanged:(CicadaPlayer*)player oldStatus:(CicadaStatus)oldStatus newStatus:(CicadaStatus)newStatus;

/**
 @brief 获取截图回调
 @param player 播放器player指针
 @param image 图像
 @see CicadaImage
 */
/****
 @brief Snapshot retrieval callback.
 @param player Player pointer.
 @param image Snapshot.
 @see CicadaImage
 */
- (void)onCaptureScreen:(CicadaPlayer*)player image:(CicadaImage*)image;

/**
 @brief SEI回调
 @param type 类型
 @param data 数据
 @see CicadaImage
 */
/****
 @brief SEI callback.
 @param type Type.
 @param data Data.
 @see CicadaImage
 */
- (void)onSEIData:(CicadaPlayer*)player type:(int)type data:(NSData *)data;

/**
 @brief 播放器渲染信息回调
 @param player 播放器player指针
 @param timeMs 渲染时的系统时间
 @param pts  视频帧pts
 */
/****
 @brief Player render info callback.
 @param player Player pointer.
 @param timeMs system time when render.
 @param pts  video frame pts
 @see AVPEventType
 */
-(void)onVideoRendered:(CicadaPlayer*)player timeMs:(int64_t)timeMs pts:(int64_t)pts;

@end

#endif /* CicadaDelegate_h */
