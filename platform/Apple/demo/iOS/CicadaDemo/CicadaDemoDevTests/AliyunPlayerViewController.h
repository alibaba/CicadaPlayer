//
//  AliyunPlayerViewController.h
//  AliPlayerAutoTest
//
//  Created by 郦立 on 2019/1/17.
//  Copyright © 2019年 wb-qxx397776. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CicadaPlayerSDK/CicadaPlayerSDK.h>

@interface AliyunPlayerViewController : UIViewController <CicadaDelegate>

@property (nonatomic,strong)CicadaPlayer *player;

@property (nonatomic, copy) void(^onPlayerEventCallback)(CicadaEventType eventType);
@property (nonatomic, copy) void(^onPlayerEventWithStringCallback)(CicadaEventWithString eventString,NSString *description);
@property (nonatomic, copy) void(^onErrorCallback)(CicadaErrorModel *errorModel);
@property (nonatomic, copy) void(^onVideoSizeChangedCallback)(int width,int height,int rotation);
@property (nonatomic, copy) void(^onCurrentPositionUpdateCallback)(int64_t position);
@property (nonatomic, copy) void(^onBufferedPositionUpdateCallback)(int64_t position);
@property (nonatomic, copy) void(^onLoadingProgressCallback)(float progress);
@property (nonatomic, copy) void(^onTrackReadyCallback)(NSArray<CicadaTrackInfo*>* info);
@property (nonatomic, copy) void(^onTrackChangedCallback)(CicadaTrackInfo* info);
@property (nonatomic, copy) void(^onSubtitleShowCallback)(int index,NSString *subtitle);
@property (nonatomic, copy) void(^onSubtitleHideCallback)(int index);
@property (nonatomic, copy) void(^onGetThumbnailSucCallback)(int64_t positionMs,int64_t fromPos,int64_t toPos,id image);
@property (nonatomic, copy) void(^onGetThumbnailFailedCallback)(int64_t positionMs);
@property (nonatomic, copy) void(^onPlayerStatusChangedCallback)(CicadaStatus oldStatus,CicadaStatus newStatus);
@property (nonatomic, copy) void(^onCaptureScreenCallback)(UIImage* image);
@property (nonatomic, copy) void(^onSELDataCallBack)(NSString * data);
@property (nonatomic, copy) void(^redrawCallback)(void);
@property (nonatomic, copy) void(^resetCallback)(void);
@property (nonatomic, copy) void(^getMediaInfoCallback)(CicadaMediaInfo* info);
@property (nonatomic, copy) void(^onSubtitleExtAdded)(int trackIndex,NSString *URL);

@property (nonatomic,strong)CicadaUrlSource *urlSource;
@property (nonatomic,strong)NSArray <CicadaTrackInfo*>*info;

- (void)initPlayer;
- (void)initPlayerWithCachePath:(NSString *)path;
- (void)destory;
- (void)pause;
- (void)redraw;
- (void)reset;
- (void)stop;
- (void)seekToTime:(int64_t)time seekMode:(CicadaSeekMode)seekMode;
- (void)snapShot;
- (void)selectTrack;
 -(void)getMediaInfo;

@end






