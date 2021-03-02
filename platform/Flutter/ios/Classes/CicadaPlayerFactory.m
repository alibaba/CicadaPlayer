//
//  VideoViewFactory.m
//  flutter_CicadaPlayer
//
//  Created by aliyun on 2020/10/9.
//
#import "CicadaPlayerFactory.h"
#import "FlutterCicadaPlayerView.h"
#import "MJExtension.h"
#import "NSDictionary+ext.h"

#define kCicadaPlayerMethod @"method"

@interface CicadaPlayerFactory () {
    NSObject<FlutterBinaryMessenger> *_messenger;
    FlutterMethodChannel *_channel;
    FlutterMethodChannel *_listPlayerchannel;
    FlutterMethodChannel *_commonChannel;
    UIView *playerView;
    NSString *mSnapshotPath;
}

@property(nonatomic, strong) FlutterEventSink eventSink;
@property(nonatomic, assign) BOOL enableMix;

@end

@implementation CicadaPlayerFactory

- (instancetype)initWithMessenger:(NSObject<FlutterBinaryMessenger> *)messenger
{
    self = [super init];
    if (self) {
        _messenger = messenger;
        __weak __typeof__(self) weakSelf = self;

        _commonChannel = [FlutterMethodChannel methodChannelWithName:@"plugins.flutter_cicadaplayer_factory" binaryMessenger:messenger];
        [_commonChannel setMethodCallHandler:^(FlutterMethodCall *call, FlutterResult result) {
          [weakSelf onMethodCall:call result:result atObj:@""];
        }];

        _channel = [FlutterMethodChannel methodChannelWithName:@"flutter_cicadaplayer" binaryMessenger:messenger];
        [_channel setMethodCallHandler:^(FlutterMethodCall *call, FlutterResult result) {
          [weakSelf onMethodCall:call result:result atObj:weakSelf.cicadaPlayer];
        }];

        FlutterEventChannel *eventChannel = [FlutterEventChannel eventChannelWithName:@"flutter_cicadaplayer_event"
                                                                      binaryMessenger:messenger];
        [eventChannel setStreamHandler:self];
    }
    return self;
}

#pragma mark - FlutterStreamHandler
- (FlutterError *_Nullable)onListenWithArguments:(id _Nullable)arguments eventSink:(FlutterEventSink)eventSink
{
    self.eventSink = eventSink;
    return nil;
}

- (FlutterError *_Nullable)onCancelWithArguments:(id _Nullable)arguments
{
    return nil;
}

- (NSObject<FlutterMessageCodec> *)createArgsCodec
{
    return [FlutterStandardMessageCodec sharedInstance];
}

- (nonnull NSObject<FlutterPlatformView> *)createWithFrame:(CGRect)frame viewIdentifier:(int64_t)viewId arguments:(id _Nullable)args
{
    FlutterCicadaPlayerView *player = [[FlutterCicadaPlayerView alloc] initWithWithFrame:frame
                                                                          viewIdentifier:viewId
                                                                               arguments:args
                                                                         binaryMessenger:_messenger];
    playerView = player.view;
    if (_cicadaPlayer) {
        _cicadaPlayer.playerView = playerView;
    }
    return player;
}

- (void)onMethodCall:(FlutterMethodCall *)call result:(FlutterResult)result atObj:(NSObject *)player
{
    NSString *method = [call method];
    SEL methodSel = NSSelectorFromString([NSString stringWithFormat:@"%@:", method]);
    NSArray *arr = @[call, result, player];
    if ([self respondsToSelector:methodSel]) {
        IMP imp = [self methodForSelector:methodSel];
        void (*func)(id, SEL, NSArray *) = (void *) imp;
        func(self, methodSel, arr);
    } else {
        result(FlutterMethodNotImplemented);
    }
}


//- (void)initService:(NSArray*)arr {
//    FlutterMethodCall* call = arr.firstObject;
//    FlutterResult result = arr[1];
//    FlutterStandardTypedData* fdata = [call arguments];
//    [CicadaPrivateService initKeyWithData:fdata.data];
//    result(nil);
//}

- (void)setUrl:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSString *url = [call arguments];
    CicadaUrlSource *source = [[CicadaUrlSource alloc] urlWithString:url];
    [player setUrlSource:source];
}

- (void)prepare:(NSArray *)arr
{
    CicadaPlayer *player = arr[2];
    [player prepare];
}

- (void)play:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    [player start];
    result(nil);
}

- (void)pause:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    [player pause];
    result(nil);
}

- (void)stop:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    [player stop];
    result(nil);
}

- (void)destroy:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    [player destroy];
    self.cicadaPlayer = nil;
    result(nil);
}

- (void)enableMix:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    FlutterResult result = arr[1];
    NSNumber *val = [call arguments];
    self.enableMix = val.boolValue;
    if (val.boolValue) {
        [CicadaPlayer setAudioSessionDelegate:self];
    } else {
        [CicadaPlayer setAudioSessionDelegate:nil];
    }
    result(nil);
}

- (void)isLoop:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    result(@([player isLoop]));
}

- (void)setLoop:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *isLoop = [call arguments];
    [player setLoop:isLoop.boolValue];
}

- (void)isAutoPlay:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    result(@([player isAutoPlay]));
}

- (void)setAutoPlay:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player setAutoPlay:val.boolValue];
}

- (void)isMuted:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    result(@([player isMuted]));
}

- (void)setMuted:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    NSNumber *val = [call arguments];
    CicadaPlayer *player = arr[2];
    [player setMuted:val.boolValue];
}

- (void)enableHardwareDecoder:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    result(@([player enableHardwareDecoder]));
}

- (void)setEnableHardwareDecoder:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player setEnableHardwareDecoder:val.boolValue];
}

- (void)getDuration:(NSArray *)arr
{
    CicadaPlayer *player = arr[2];
    FlutterResult result = arr[1];
    result(@(player.duration));
}

- (void)getRotateMode:(NSArray *)arr
{
    CicadaPlayer *player = arr[2];
    FlutterResult result = arr[1];
    result(@(player.rotateMode));
}

- (void)setRotateMode:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player setRotateMode:val.intValue];
}

- (void)getScalingMode:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    int mode = 0;
    switch (player.scalingMode) {
        case CICADA_SCALINGMODE_SCALEASPECTFIT:
            mode = 0;
            break;
        case CICADA_SCALINGMODE_SCALEASPECTFILL:
            mode = 1;
            break;
        case CICADA_SCALINGMODE_SCALETOFILL:
            mode = 2;
            break;

        default:
            break;
    }
    result(@(mode));
}

- (void)setScalingMode:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    //    与android保持一致
    int mode = CICADA_SCALINGMODE_SCALEASPECTFIT;
    switch (val.intValue) {
        case 0:
            mode = CICADA_SCALINGMODE_SCALEASPECTFIT;
            break;
        case 1:
            mode = CICADA_SCALINGMODE_SCALEASPECTFILL;
            break;
        case 2:
            mode = CICADA_SCALINGMODE_SCALETOFILL;
            break;

        default:
            break;
    }
    [player setScalingMode:mode];
    result(nil);
}

- (void)getMirrorMode:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    result(@(player.mirrorMode));
}

- (void)setMirrorMode:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player setMirrorMode:val.intValue];
}

- (void)getRate:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    result(@(player.rate));
}

- (void)setRate:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player setRate:val.floatValue];
}

- (void)snapshot:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSString *val = [call arguments];
    mSnapshotPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    if (val.length > 0) {
        mSnapshotPath = [mSnapshotPath stringByAppendingPathComponent:val];
    }
    [player snapShot];
}

- (void)createThumbnailHelper:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSString *val = [call arguments];
    [player setThumbnailUrl:val];
    self.eventSink(@{kCicadaPlayerMethod: @"thumbnail_onPrepared_Success"});
}

- (void)requestBitmapAtPosition:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player getThumbnail:val.integerValue];
}

- (void)getVolume:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    result(@(player.volume));
}

- (void)setVolume:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player setVolume:val.floatValue];
}

- (void)setVideoBackgroundColor:(NSArray *)arr
{
    //    FlutterMethodCall* call = arr.firstObject;
    //    CicadaPlayer *player = arr[2];
    //    NSNumber* val = [call arguments];
    //    int c = val.intValue;
    //    UIColor *color = [UIColor colorWithRed:((c>>16)&0xFF)/255.0 green:((c>>8)&0xFF)/255.0 blue:((c)&0xFF)/255.0  alpha:((c>>24)&0xFF)/255.0];
    //    [player setVideoBackgroundColor:color];
}

- (void)getSDKVersion:(NSArray *)arr
{
    FlutterResult result = arr[1];
    result([CicadaPlayer getSDKVersion]);
}

- (void)enableConsoleLog:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSNumber *val = [call arguments];
    [player setEnableLog:val.boolValue];
}

- (void)getLogLevel:(NSArray *)arr
{
    FlutterResult result = arr[1];
    //TODO 拿不到
    result(@(-1));
}

- (void)setLogLevel:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    NSNumber *val = [call arguments];
    [CicadaPlayer setLogCallbackInfo:val.intValue callbackBlock:nil];
}

- (void)seekTo:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSDictionary *dic = [call arguments];
    NSNumber *position = dic[@"position"];
    NSNumber *seekMode = dic[@"seekMode"];
    [player seekToTime:position.integerValue seekMode:seekMode.intValue];
}

//TODO 应该是根据已经有的key 替换比较合理
- (void)setConfig:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSDictionary *val = [call arguments];
    CicadaConfig *config = [player getConfig];

    [CicadaConfig mj_setupReplacedKeyFromPropertyName:^NSDictionary * {
      return @{
          @"httpProxy": @"mHttpProxy",
          @"referer": @"mReferrer",
          @"networkTimeout": @"mNetworkTimeout",
          @"highBufferDuration": @"mHighBufferDuration",
          @"maxDelayTime": @"mMaxDelayTime",
          @"maxBufferDuration": @"mMaxBufferDuration",
          @"startBufferDuration": @"mStartBufferDuration",
          @"maxProbeSize": @"mMaxProbeSize",
          @"maxProbeSize": @"mMaxProbeSize",
          @"clearShowWhenStop": @"mClearFrameWhenStop",
          @"enableVideoTunnelRender": @"mEnableVideoTunnelRender",
          @"enableSEI": @"mEnableSEI",
          @"userAgent": @"mUserAgent",
          @"networkRetryCount": @"mNetworkRetryCount",
          @"liveStartIndex": @"mLiveStartIndex",
          @"customHeaders": @"mCustomHeaders",
          @"disableAudio": @"mDisableAudio",
          @"disableVideo": @"mDisableVideo",
      };
    }];

    config = [CicadaConfig mj_objectWithKeyValues:val];

    [player setConfig:config];
}

//- (void)getCacheConfig:(NSArray*)arr {
//    FlutterResult result = arr[1];
//    CicadaPlayer *player = arr[2];
//    [CicadaCacheConfig mj_setupReplacedKeyFromPropertyName:^NSDictionary *{
//        return @{
//                 @"enable" : @"mEnable",
//                 @"path" :@"mDir",
//                 @"maxSizeMB" :@"mMaxSizeMB",
//                 @"maxDuration" :@"mMaxDurationS",
//                 };
//    }];
//    result(config.mj_keyValues);
//}

- (void)setCacheConfig:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSDictionary *val = [call arguments];

    [CicadaCacheConfig mj_setupReplacedKeyFromPropertyName:^NSDictionary * {
      return @{
          @"enable": @"mEnable",
          @"path": @"mDir",
          @"maxSizeMB": @"mMaxSizeMB",
          @"maxDuration": @"mMaxDurationS",
      };
    }];
    CicadaCacheConfig *config = [CicadaCacheConfig mj_objectWithKeyValues:val];
    NSString *path = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES).firstObject;
    [config setPath:[path stringByAppendingPathComponent:config.path]];

    [player setCacheConfig:config];
}

- (void)getConfig:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    CicadaConfig *config = [player getConfig];

    [CicadaConfig mj_setupReplacedKeyFromPropertyName:^NSDictionary * {
      return @{
          @"httpProxy": @"mHttpProxy",
          @"referer": @"mReferrer",
          @"networkTimeout": @"mNetworkTimeout",
          @"highBufferDuration": @"mHighBufferDuration",
          @"maxDelayTime": @"mMaxDelayTime",
          @"maxBufferDuration": @"mMaxBufferDuration",
          @"startBufferDuration": @"mStartBufferDuration",
          @"maxProbeSize": @"mMaxProbeSize",
          @"maxProbeSize": @"mMaxProbeSize",
          @"clearShowWhenStop": @"mClearFrameWhenStop",
          @"enableVideoTunnelRender": @"mEnableVideoTunnelRender",
          @"enableSEI": @"mEnableSEI",
          @"userAgent": @"mUserAgent",
          @"networkRetryCount": @"mNetworkRetryCount",
          @"liveStartIndex": @"mLiveStartIndex",
          @"customHeaders": @"mCustomHeaders",
      };
    }];
    result(config.mj_keyValues);
}

- (void)getMediaInfo:(NSArray *)arr
{
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    CicadaMediaInfo *info = [player getMediaInfo];

    //TODO 后面需要统一键值转换规则
    [CicadaMediaInfo mj_setupReplacedKeyFromPropertyName:^NSDictionary * {
      return @{
          @"mTitle": @"title",
          @"mCoverUrl": @"coverURL",
          @"mTrackInfos": @"tracks",
      };
    }];

    [CicadaTrackInfo mj_setupReplacedKeyFromPropertyName:^NSDictionary * {
      return @{
          @"vodDefinition": @"trackDefinition",
          @"index": @"trackIndex",
      };
    }];

    [CicadaThumbnailInfo mj_setupReplacedKeyFromPropertyName:^NSDictionary * {
      return @{
          @"URL": @"url",
      };
    }];
    NSLog(@"getMediaInfo==%@", info.mj_JSONString);
    result(info.mj_keyValues);
}

- (void)getCurrentTrack:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    NSNumber *idxNum = call.arguments;
    CicadaTrackInfo *info = [player getCurrentTrack:idxNum.intValue];
    NSLog(@"getCurrentTrack==%@", info.mj_JSONString);
    result(info.mj_keyValues);
}

- (void)selectTrack:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSDictionary *dic = [[call arguments] removeNull];
    NSNumber *trackIdxNum = dic[@"trackIdx"];
    NSNumber *accurateNum = dic[@"accurate"];
    if (accurateNum.intValue == -1) {
        [player selectTrack:trackIdxNum.intValue];
    } else {
        //        [player selectTrack:trackIdxNum.intValue accurate:accurateNum.boolValue];
    }
}

- (void)addExtSubtitle:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    CicadaPlayer *player = arr[2];
    NSString *url = [call arguments];
    [player addExtSubtitle:url];
}

- (void)selectExtSubtitle:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    NSDictionary *dic = [[call arguments] removeNull];
    NSNumber *trackIdxNum = dic[@"trackIndex"];
    NSNumber *enableNum = dic[@"enable"];
    [player selectExtSubtitle:trackIdxNum.intValue enable:enableNum.boolValue];
    result(nil);
}

- (void)setStreamDelayTime:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    NSDictionary *dic = [[call arguments] removeNull];
    NSNumber *trackIdxNum = dic[@"index"];
    NSNumber *timeNum = dic[@"time"];
    [player setStreamDelayTime:trackIdxNum.intValue time:timeNum.intValue];
    result(nil);
}

- (void)setPreferPlayerName:(NSArray *)arr
{
    FlutterMethodCall *call = arr.firstObject;
    FlutterResult result = arr[1];
    CicadaPlayer *player = arr[2];
    //    NSString *playerName = [call arguments];
    //    [player setPreferPlayerName:playerName];
    result(nil);
}

- (void)getPlayerName:(NSArray *)arr
{
    //    FlutterResult result = arr[1];
    //    CicadaPlayer *player = arr[2];
    //    result([player getPlayerName]);
}

#pragma-- mark getters
- (CicadaPlayer *)cicadaPlayer
{
    if (!_cicadaPlayer) {
        _cicadaPlayer = [[CicadaPlayer alloc] init];
        _cicadaPlayer.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFIT;
        _cicadaPlayer.rate = 1;
        _cicadaPlayer.delegate = self;
        _cicadaPlayer.playerView = playerView;
    }
    return _cicadaPlayer;
}


#pragma mark CicadaDelegate

/**
 @brief 播放器状态改变回调
 @param player 播放器player指针
 @param oldStatus 老的播放器状态 参考CicadaStatus
 @param newStatus 新的播放器状态 参考CicadaStatus
 */
- (void)onPlayerStatusChanged:(CicadaPlayer *)player oldStatus:(CicadaStatus)oldStatus newStatus:(CicadaStatus)newStatus
{
    self.eventSink(@{kCicadaPlayerMethod: @"onStateChanged", @"newState": @(newStatus)});
}

/**
 @brief 错误代理回调
 @param player 播放器player指针
 @param errorModel 播放器错误描述，参考AliVcPlayerErrorModel
 */
- (void)onError:(CicadaPlayer *)player errorModel:(CicadaErrorModel *)errorModel
{
    self.eventSink(@{kCicadaPlayerMethod: @"onError", @"errorCode": @(errorModel.code), @"errorMsg": errorModel.message});
}

- (void)onSEIData:(CicadaPlayer *)player type:(int)type data:(NSData *)data
{
    NSString *str = [NSString stringWithUTF8String:data.bytes];
    NSLog(@"SEI: %@", str);
}

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventType 播放器事件类型，@see CicadaEventType
 */
- (void)onPlayerEvent:(CicadaPlayer *)player eventType:(CicadaEventType)eventType
{
    switch (eventType) {
        case CicadaEventPrepareDone:
            self.eventSink(@{kCicadaPlayerMethod: @"onPrepared"});
            break;
        case CicadaEventFirstRenderedStart:
            self.eventSink(@{kCicadaPlayerMethod: @"onRenderingStart"});
            break;
        case CicadaEventLoadingStart:
            self.eventSink(@{kCicadaPlayerMethod: @"onLoadingBegin"});
            break;
        case CicadaEventLoadingEnd:
            self.eventSink(@{kCicadaPlayerMethod: @"onLoadingEnd"});
            break;
        case CicadaEventCompletion:
            self.eventSink(@{kCicadaPlayerMethod: @"onCompletion"});
            break;
        case CicadaEventSeekEnd:
            self.eventSink(@{kCicadaPlayerMethod: @"onSeekComplete"});
            break;
        default:
            break;
    }
}

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventWithString 播放器事件类型
 @param description 播放器事件说明
 @see CicadaEventType
 */
- (void)onPlayerEvent:(CicadaPlayer *)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description
{
    self.eventSink(@{kCicadaPlayerMethod: @"onInfo", @"infoCode": @(eventWithString), @"extraMsg": description});
}

/**
 @brief 视频当前播放位置回调
 @param player 播放器player指针
 @param position 视频当前播放位置
 */
- (void)onCurrentPositionUpdate:(CicadaPlayer *)player position:(int64_t)position
{
    self.eventSink(@{kCicadaPlayerMethod: @"onInfo", @"infoCode": @(2), @"extraValue": @(position)});
}

/**
 @brief 视频缓存位置回调
 @param player 播放器player指针
 @param position 视频当前缓存位置
 */
- (void)onBufferedPositionUpdate:(CicadaPlayer *)player position:(int64_t)position
{
    self.eventSink(@{kCicadaPlayerMethod: @"onInfo", @"infoCode": @(1), @"extraValue": @(position)});
}

/**
 @brief 获取track信息回调
 @param player 播放器player指针
 @param info track流信息数组 参考CicadaTrackInfo
 */
- (void)onTrackReady:(CicadaPlayer *)player info:(NSArray<CicadaTrackInfo *> *)info
{
    self.eventSink(@{kCicadaPlayerMethod: @"onTrackReady"});
}

/**
 @brief 外挂字幕被添加
 @param player 播放器player指针
 @param trackIndex 字幕显示的索引号
 @param URL 字幕url
 */
- (void)onSubtitleExtAdded:(CicadaPlayer *)player trackIndex:(int)trackIndex URL:(NSString *)URL
{
    self.eventSink(@{kCicadaPlayerMethod: @"onSubtitleExtAdded", @"trackIndex": @(trackIndex), @"url": URL});
}

/**
 @brief 字幕显示回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 @param subtitle 字幕显示的字符串
 */
- (void)onSubtitleShow:(CicadaPlayer *)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID subtitle:(NSString *)subtitle
{
    self.eventSink(
            @{kCicadaPlayerMethod: @"onSubtitleShow", @"trackIndex": @(trackIndex), @"subtitleID": @(subtitleID), @"subtitle": subtitle});
}

/**
 @brief 字幕隐藏回调
 @param player 播放器player指针
 @param trackIndex 字幕流索引.
 @param subtitleID  字幕ID.
 */
- (void)onSubtitleHide:(CicadaPlayer *)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID
{
    self.eventSink(@{kCicadaPlayerMethod: @"onSubtitleHide", @"trackIndex": @(trackIndex), @"subtitleID": @(subtitleID)});
}

/**
 @brief 获取截图回调
 @param player 播放器player指针
 @param image 图像
 */
- (void)onCaptureScreen:(CicadaPlayer *)player image:(UIImage *)image
{
    BOOL result = [UIImagePNGRepresentation(image) writeToFile:mSnapshotPath atomically:YES];// 保存成功会返回YES
    if (result == YES) {
        self.eventSink(@{kCicadaPlayerMethod: @"onSnapShot", @"snapShotPath": mSnapshotPath});
    }
}

/**
 @brief track切换完成回调
 @param player 播放器player指针
 @param info 切换后的信息 参考CicadaTrackInfo
 */
- (void)onTrackChanged:(CicadaPlayer *)player info:(CicadaTrackInfo *)info
{
    NSLog(@"onTrackChanged==%@", info.mj_JSONString);
    self.eventSink(@{kCicadaPlayerMethod: @"onTrackChanged", @"info": info.mj_keyValues});
}

/**
 @brief 获取缩略图成功回调
 @param positionMs 指定的缩略图位置
 @param fromPos 此缩略图的开始位置
 @param toPos 此缩略图的结束位置
 @param image 缩图略图像指针,对于mac是NSImage，iOS平台是UIImage指针
 */
- (void)onGetThumbnailSuc:(int64_t)positionMs fromPos:(int64_t)fromPos toPos:(int64_t)toPos image:(id)image
{
    NSData *imageData = UIImageJPEGRepresentation(image, 1);
    //    FlutterStandardTypedData * fdata = [FlutterStandardTypedData typedDataWithBytes:imageData];
    self.eventSink(
            @{kCicadaPlayerMethod: @"onThumbnailGetSuccess", @"thumbnailRange": @[@(fromPos), @(toPos)], @"thumbnailbitmap": imageData});
}

/**
 @brief 获取缩略图失败回调
 @param positionMs 指定的缩略图位置
 */
- (void)onGetThumbnailFailed:(int64_t)positionMs
{
    self.eventSink(@{kCicadaPlayerMethod: @"onThumbnailGetFail"});
}

/**
 @brief 视频缓冲进度回调
 @param player 播放器player指针
 @param progress 缓存进度0-100
 */
- (void)onLoadingProgress:(CicadaPlayer *)player progress:(float)progress
{
    self.eventSink(@{kCicadaPlayerMethod: @"onLoadingProgress", @"percent": @((int) progress)});
}


#pragma-- mark CicadaAudioSessionDelegate
- (BOOL)setActive:(BOOL)active error:(NSError **)outError
{
    return [[AVAudioSession sharedInstance] setActive:active error:outError];
}

- (BOOL)setCategory:(NSString *)category withOptions:(AVAudioSessionCategoryOptions)options error:(NSError **)outError
{
    if (self.enableMix) {
        options = AVAudioSessionCategoryOptionMixWithOthers | AVAudioSessionCategoryOptionDuckOthers;
    }
    return [[AVAudioSession sharedInstance] setCategory:category withOptions:options error:outError];
}

- (BOOL)setCategory:(AVAudioSessionCategory)category
                      mode:(AVAudioSessionMode)mode
        routeSharingPolicy:(AVAudioSessionRouteSharingPolicy)policy
                   options:(AVAudioSessionCategoryOptions)options
                     error:(NSError **)outError
{
    if (self.enableMix) {
        return YES;
    }

    if (@available(iOS 11.0, tvOS 11.0, *)) {
        return [[AVAudioSession sharedInstance] setCategory:category mode:mode routeSharingPolicy:policy options:options error:outError];
    }
    return NO;
}

@end
