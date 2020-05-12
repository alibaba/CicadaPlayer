//
//  CicadaPlayerViewController.m
//  CicadaPlayerDemo
//
//  Created by 郦立 on 2019/1/2.
//  Copyright © 2019年 com.alibaba. All rights reserved.
//

#import "CicadaPlayerViewController.h"
#import "CicadaDemoView.h"
#import "CicadaSettingAndConfigView.h"
#import "UIView+AVPFrame.h"
#import "CicadaConfig+refresh.h"
#import "CicadaCacheConfig+refresh.h"
#import "AppDelegate.h"
#import "UIViewController+backAction.h"
#import "CicadaShowImageView.h"
#import "CicadaErrorModel+string.h"
#import "AFNetworking.h"

@interface CicadaPlayerViewController ()<CicadaDemoViewDelegate,CicadaSettingAndConfigViewDelegate,CicadaDelegate,CicadaAudioSessionDelegate, CicadaRenderDelegate>

/**
 播放视图
 */
@property (nonatomic,strong)CicadaDemoView *CicadaView;

/**
 底部tab选择视图
 */
@property (nonatomic,strong)CicadaSettingAndConfigView *settingAndConfigView;

/**
 播放器
 */
@property (nonatomic,strong)CicadaPlayer *player;

/**
 当前Track是否有缩略图，如果没有，不展示缩略图
 */
@property (nonatomic,assign)BOOL trackHasThumbnai;

/**
 记录当前网络类型
 */
@property (nonatomic,assign)AFNetworkReachabilityStatus currentNetworkStatus;

/**
 记录流量网络是否重试
 */
@property (nonatomic,assign)BOOL wanWillRetry;

/**
 剩余重试次数
*/
@property (nonatomic,assign)NSInteger retryCount;

/**
 记录是否完成viewdidload,防止还没完成添加，返回界面，移除空观察者造成的崩溃
 */
@property (nonatomic,assign)BOOL isViewDidLoad;

/**
当前的外挂字幕的Index
*/
@property (nonatomic,assign)int extSubtitleTrackIndex;

/**
点击准备时，需要调用stop
*/
@property (nonatomic,assign)BOOL needStop;

/**
混音播放
*/
@property (nonatomic,assign)BOOL enableMix;

@end

@implementation CicadaPlayerViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    
    self.retryCount = 3;
    [self setScreenCanRotation:YES];
    self.view.backgroundColor = [UIColor whiteColor];
    self.navigationItem.title = NSLocalizedString(@"播放" , nil);
    
    //禁止手势返回
    if ([self.navigationController respondsToSelector:@selector(interactivePopGestureRecognizer)]) {
        self.navigationController.interactivePopGestureRecognizer.enabled = NO;
    }
    
    self.CicadaView = [[CicadaDemoView alloc]initWithFrame:CGRectMake(0, NAVIGATION_HEIGHT, SCREEN_WIDTH, SCREEN_WIDTH/16*9+44)];
    self.CicadaView.delegate = self;
    [self.view addSubview:self.CicadaView];

    self.settingAndConfigView = [[CicadaSettingAndConfigView alloc]initWithFrame:CGRectMake(0, self.CicadaView.getMaxY, SCREEN_WIDTH, SCREEN_HEIGHT - self.CicadaView.getMaxY - SAFE_BOTTOM)];
    self.settingAndConfigView.delegate = self;
    NSMutableArray *configArray = [CicadaTool getCicadaConfigArray];
    [self.settingAndConfigView setIshardwareDecoder:[CicadaTool isHardware]];
    [self.settingAndConfigView setConfigArray:configArray];
    [self.view addSubview:self.settingAndConfigView];

    [CicadaPlayer setAudioSessionDelegate:self];
    self.player = [[CicadaPlayer alloc] init];
    self.player.enableHardwareDecoder = [CicadaTool isHardware];
    self.player.playerView = self.CicadaView.playerView;
    self.player.delegate = self;
    //enable to test render delegate
//    self.player.renderDelegate = self;
    self.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFIT;
    [self.settingAndConfigView setVolume:self.player.volume/2];
    [self setConfig];
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationDidChangeFunc) name:UIDeviceOrientationDidChangeNotification object:nil];
    // 添加检测app进入后台的观察者
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationEnterBackground) name: UIApplicationWillResignActiveNotification object:nil];
    // app从后台进入前台都会调用这个方法
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(applicationDidBecomeActive) name: UIApplicationDidBecomeActiveNotification object:nil];
    
    WEAK_SELF
    [[AFNetworkReachabilityManager sharedManager] startMonitoring];
    [[AFNetworkReachabilityManager sharedManager] setReachabilityStatusChangeBlock:^(AFNetworkReachabilityStatus status) {
        if (weakSelf.retryCount == 0) {
            [weakSelf.player reload];
        }else if (status == AFNetworkReachabilityStatusReachableViaWWAN) {
            //切换到流量
            UIAlertController *alert = [UIAlertController alertControllerWithTitle:nil message:NSLocalizedString(@"当前为流量网络，是否继续?" , nil) preferredStyle:UIAlertControllerStyleAlert];
            UIAlertAction *sureAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"确认" , nil) style:UIAlertActionStyleDefault handler:^(UIAlertAction * _Nonnull action) {
                weakSelf.wanWillRetry = YES;
            }];
            [alert addAction:sureAction];
            UIAlertAction *cancelAction = [UIAlertAction actionWithTitle:NSLocalizedString(@"取消" , nil) style:UIAlertActionStyleCancel handler:^(UIAlertAction * _Nonnull action) {
                weakSelf.wanWillRetry = NO;
            }];
            [alert addAction:cancelAction];
            [weakSelf presentViewController:alert animated:YES completion:nil];
        }
        weakSelf.currentNetworkStatus = status;
    }];

    [self.player addObserver:self
                  forKeyPath:@"width"
                     options:NSKeyValueObservingOptionNew
                     context:nil];
    [self.player addObserver:self
                  forKeyPath:@"height"
                     options:NSKeyValueObservingOptionNew
                     context:nil];
    self.isViewDidLoad = YES;
}

// whenever an observed key path changes, this method will be called
- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context
{
    NSNumber* value = [self.player valueForKeyPath:keyPath];
    NSLog(@"keyPath:%@, value:%@, change:%@", keyPath, value, change);
}

- (void)viewDidDisappear:(BOOL)animated {
    [super viewDidDisappear:animated];

    if (self.isViewDidLoad) {
        [self.player removeObserver:self forKeyPath:@"width"];
        [self.player removeObserver:self forKeyPath:@"height"];
    }
    [self.player stop];
    [self.player destroy];
    [CicadaPlayer setAudioSessionDelegate:nil];
    [self setScreenCanRotation:NO];
}

- (void)setScreenCanRotation:(BOOL)canRotation {
    AppDelegate *deledage = (AppDelegate *)[UIApplication sharedApplication].delegate;
    deledage.allowRotation = canRotation;
}

- (void)orientationDidChangeFunc {
    if (IS_PORTRAIT) {
        UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
        bool iphonexLeft = (orientation == UIDeviceOrientationLandscapeLeft || orientation == UIDeviceOrientationLandscapeRight);
        if (IS_IPHONEX && iphonexLeft) {
            NSNumber *value = [NSNumber numberWithInt:UIInterfaceOrientationPortrait];
            [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
        }else {
            self.settingAndConfigView.hidden = NO;
        }
        self.navigationController.navigationBar.hidden = NO;
    }else {
        self.settingAndConfigView.hidden = YES;
        self.navigationController.navigationBar.hidden = YES;
    }
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation {
    [self.player redraw];
}

- (void)applicationEnterBackground {
    if (!self.settingAndConfigView.isPlayBackgournd) {
        [self.player pause];
    }
}

- (void)applicationDidBecomeActive {
    if (!self.settingAndConfigView.isPlayBackgournd) {
        [self.player start];
    }
}

#pragma mark navigationPopback

/**
 返回键点击事件

 @return 是否可以返回
 */
- (BOOL)navigationShouldPopOnBackButton{
    //如果竖屏可以返回，如果横屏，先竖屏幕
    if (IS_PORTRAIT) {
        return YES;
    }
    NSNumber *value = [NSNumber numberWithInt:UIInterfaceOrientationPortrait];
    [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
    return NO;
}

#pragma mark CicadaDemoViewDelegate

/**
 底部按钮点击回调
 
 @param playerView playerView
 @param index 0:准备,1:播放,2:暂停,3:停止,4:截图,5:重试
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView bottonButtonClickAtIndex:(NSInteger)index {
    NSLog(@"%ld",(long)index);
    switch (index) {
        case 0: {
            [self.settingAndConfigView setIshardwareDecoder:[CicadaTool isHardware]];
            if (self.needStop) {
                [self.player stop];
            }
            self.needStop = YES;
            for (NSString *value in self.subtitleDictionary.allValues) {
                [self.player addExtSubtitle:value];
            }
            if (self.urlSource) {
                [self.player setUrlSource:self.urlSource];
                [self.player prepare];
            }
        }
            break;
        case 1: { [self.player start]; }
            break;
        case 2: { [self.player pause]; }
            break;
        case 3: {
            [self.player stop];
            self.needStop = NO;
            [self.CicadaView hiddenLoadingView];
            self.CicadaView.subTitleLabel.hidden = YES;
            self.CicadaView.bufferPosition = 0;
            self.CicadaView.currentPosition = 0;
            self.CicadaView.allPosition = 0;
        }
            break;
        case 4: {
            [self.player snapShot];
        }
            break;
        case 5: {
            [self.player reload];
        }
            break;
        default:
            break;
    }
}

/**
 全屏按钮点击回调
 
 @param playerView playerView
 @param isFull 是否全屏
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView fullScreenButtonAction:(BOOL)isFull {
    NSLog(@"%d",isFull);
}

/**
 进度条完成进度回调
 
 @param playerView playerView
 @param value 进度值
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView progressSliderDidChange:(CGFloat)value {
    NSLog(@"%f",value);
    [self.CicadaView hiddenThumbnailView];
    
    CicadaSeekMode seekMode = CICADA_SEEKMODE_INACCURATE;
    if (self.settingAndConfigView.isAccurateSeek) { seekMode = CICADA_SEEKMODE_ACCURATE; }
    [self.player seekToTime:value*self.player.duration seekMode:seekMode];
}

/**
 进度条改变进度回调
 
 @param playerView playerView
 @param value 进度值
 */
- (void)CicadaDemoView:(CicadaDemoView *)playerView progressSliderValueChanged:(CGFloat)value {
    if (self.trackHasThumbnai) {
        [self.player getThumbnail:self.player.duration*value];
    }
}

#pragma mark CicadaSettingAndConfigViewDelegate

/**
 switch按钮点击回调
 
 @param view settingAndConfigView
 @param index 0:自动播放,1:静音,2:循环,3:硬解码,4:精准seek
 @param isOpen 是否打开
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view switchChangedIndex:(NSInteger)index isOpen:(BOOL)isOpen {
    NSLog(@"%ld %d",(long)index,isOpen);
    switch (index) {
        case 0: { self.player.autoPlay = isOpen; }
            break;
        case 1: { self.player.muted = isOpen; }
            break;
        case 2: { self.player.loop = isOpen; }
            break;
        case 3: { self.player.enableHardwareDecoder = isOpen; }
            break;
        default:
            break;
    }
}

/**
 声音进度条点击回调
 
 @param view settingAndConfigView
 @param value 进度值
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view voiceSliderDidChange:(CGFloat)value {
    NSLog(@"%f",value);
    self.player.muted = NO;
    self.player.volume = value * 2;
}

/**
 segmented点击回调
 
 @param view settingAndConfigView
 @param index 0:缩放模式,1:镜像模式,2:旋转模式,3:倍速播放
 @param selectedIndex 选择了第几个seg
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view segmentedControlIndex:(NSInteger)index selectedIndex:(NSInteger)selectedIndex {
    NSLog(@"%ld %ld",(long)index,(long)selectedIndex);
    switch (index) {
        case 0: {
            switch (selectedIndex) {
                case 0: { self.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFIT; }
                    break;
                case 1: { self.player.scalingMode = CICADA_SCALINGMODE_SCALEASPECTFILL; }
                    break;
                case 2: { self.player.scalingMode = CICADA_SCALINGMODE_SCALETOFILL; }
                    break;
                default:
                    break;
            }
        }
            break;
        case 1: {
            switch (selectedIndex) {
                case 0: { self.player.mirrorMode = CICADA_MIRRORMODE_NONE; }
                    break;
                case 1: { self.player.mirrorMode = CICADA_MIRRORMODE_HORIZONTAL; }
                    break;
                case 2: { self.player.mirrorMode = CICADA_MIRRORMODE_VERTICAL; }
                    break;
                default:
                    break;
            }
        }
            break;
        case 2: {
            switch (selectedIndex) {
                case 0: { self.player.rotateMode = CICADA_ROTATE_0; }
                    break;
                case 1: { self.player.rotateMode = CICADA_ROTATE_90; }
                    break;
                case 2: { self.player.rotateMode = CICADA_ROTATE_180; }
                    break;
                case 3: { self.player.rotateMode = CICADA_ROTATE_270; }
                    break;
                default:
                    break;
            }
        }
            break;
        case 3: {
            switch (selectedIndex) {
                case 0: { self.player.rate = 1; }
                    break;
                case 1: { self.player.rate = 0.5; }
                    break;
                case 2: { self.player.rate = 1.5; }
                    break;
                case 3: { self.player.rate = 2; }
                    break;
                default:
                    break;
            }
        }
            break;
        default:
            break;
    }
}

- (void)setConfig {
    NSArray *configArray = [self.settingAndConfigView getConfigArray];
    CicadaConfig *config = [self.player getConfig];
    [config refreshConfigWithArray:configArray];
    [self.player setConfig:config];
}

- (void)setCacheConfig {
    NSDictionary *cacheDic = [self.settingAndConfigView getCacheConfigDictionary];
    CicadaCacheConfig *config = [[CicadaCacheConfig alloc]init];
    [config refreshConfigWithDictionary:cacheDic];
    [self.player setCacheConfig:config];
}

/**
 底部按钮点击回调
 
 @param view settingAndConfigView
 @param index 0:媒体信息,1:刷新配置,2:cache刷新配置
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view bottonButtonClickIndex:(NSInteger)index {
    NSLog(@"%ld",(long)index);
    switch (index) {
        case 0: {
            NSMutableString *infoString = [NSMutableString string];
            CicadaTrackInfo *videoTrack = [self.player getCurrentTrack:CICADA_TRACK_VIDEO];
            CicadaTrackInfo *audioTrack = [self.player getCurrentTrack:CICADA_TRACK_AUDIO];
            CicadaTrackInfo *subtitleTrack = [self.player getCurrentTrack:CICADA_TRACK_SUBTITLE];
            if (videoTrack > 0) {
                [infoString appendString:NSLocalizedString(@"清晰度:" , nil)];
                [infoString appendString:[NSString stringWithFormat:@"%d",videoTrack.trackBitrate]];
                [infoString appendString:@"; "];
            }
            if (audioTrack) {
                [infoString appendString:NSLocalizedString(@"音轨:" , nil)];
                [infoString appendString:audioTrack.description];
                [infoString appendString:@"; "];
            }
            if (subtitleTrack) {
                [infoString appendString:NSLocalizedString(@"字幕:" , nil)];
                [infoString appendString:subtitleTrack.description];
                [infoString appendString:@"; "];
            }
            if (infoString.length == 0) {
                [CicadaTool hudWithText:NSLocalizedString(@"媒体信息暂缺" , nil) view:self.view];
            }else {
                [CicadaTool hudWithText:infoString.copy view:self.view];
            }
        }
            break;
        case 1: {
            [self setConfig];
            [CicadaTool hudWithText:NSLocalizedString(@"使用成功" , nil) view:self.view];
        }
            break;
        case 2: {
            [self setCacheConfig];
            [CicadaTool hudWithText:NSLocalizedString(@"使用成功" , nil) view:self.view];
        }
            break;
        default:
            break;
    }
}

/**
 tableview点击回调
 
 @param view settingAndConfigView
 @param info 选择的track
 */
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view tableViewDidSelectTrack:(CicadaTrackInfo *)info {
    [self.player selectTrack:info.trackIndex];
}

/**
tableview点击外挂字幕回调

@param view settingAndConfigView
@param index 选择的index
@param key 选择的键
*/
- (void)settingAndConfigView:(CicadaSettingAndConfigView *)view tableViewSelectSubtitle:(int)index subtitleKey:(NSString *)key {
    if (self.extSubtitleTrackIndex != index) {
        [self.player selectExtSubtitle:self.extSubtitleTrackIndex enable:NO];
        [self.player selectExtSubtitle:index enable:YES];
        self.extSubtitleTrackIndex = index;
        [CicadaTool hudWithText:[NSString stringWithFormat:@"%@%@",NSLocalizedString(@"打开外挂字幕" , nil),key] view:self.view];
    }else {
        [self.player selectExtSubtitle:self.extSubtitleTrackIndex enable:NO];
        self.extSubtitleTrackIndex = -999;
        [CicadaTool hudWithText:[NSString stringWithFormat:@"%@%@",NSLocalizedString(@"关闭外挂字幕" , nil),key] view:self.view];
    }
}

#pragma mark CicadaDelegate

/**
 @brief 错误代理回调
 @param player 播放器player指针
 @param errorModel 播放器错误描述，参考CicadaErrorModel
 */
- (void)onError:(CicadaPlayer*)player errorModel:(CicadaErrorModel *)errorModel {
    [CicadaTool showAlert:[errorModel errorString] sender:self];
    [self.CicadaView hiddenLoadingView];
    [self.player stop];
    self.CicadaView.bufferPosition = 0;
    self.CicadaView.currentPosition = 0;
    self.CicadaView.allPosition = 0;
    [self.settingAndConfigView resetTableViewData];
}

/**
 @brief 播放器事件回调
 @param player 播放器player指针
 @param eventType 播放器事件类型，@see CicadaEventType
 */
-(void)onPlayerEvent:(CicadaPlayer*)player eventType:(CicadaEventType)eventType {
    switch (eventType) {
        case CicadaEventPrepareDone: {
            if (self.player.duration == 0) {
                self.CicadaView.hiddenSlider = YES;
            }else {
                self.CicadaView.hiddenSlider = NO;
                self.CicadaView.allPosition = self.player.duration;
            }
            CicadaTrackInfo *videoInfo = [self.player getCurrentTrack:CICADA_TRACK_VIDEO];
            NSString *bitrate = [NSString stringWithFormat:@"%d", videoInfo.trackBitrate];
            [self.settingAndConfigView setCurrentVideo:bitrate];
            [CicadaTool hudWithText:NSLocalizedString(@"准备完成" , nil) view:self.view];
        }
            break;
        case CicadaEventAutoPlayStart:
            break;
        case CicadaEventFirstRenderedStart:
            [CicadaTool hudWithText:NSLocalizedString(@"首帧显示" , nil) view:self.view];
            break;
        case CicadaEventCompletion:
            [CicadaTool hudWithText:NSLocalizedString(@"播放完成" , nil) view:self.view];
            break;
        case CicadaEventLoadingStart: {
            [self.CicadaView showLoadingView];
            [CicadaTool hudWithText:NSLocalizedString(@"缓冲开始" , nil) view:self.view];
        }
            break;
        case CicadaEventLoadingEnd: {
            [self.CicadaView hiddenLoadingView];
            [CicadaTool hudWithText:NSLocalizedString(@"缓冲完成" , nil) view:self.view];
        }
            break;
        case CicadaEventSeekEnd:
            [CicadaTool hudWithText:NSLocalizedString(@"跳转完成" , nil) view:self.view];
            break;
        case CicadaEventLoopingStart:
            [CicadaTool hudWithText:NSLocalizedString(@"循环播放开始" , nil) view:self.view];
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
-(void)onPlayerEvent:(CicadaPlayer*)player eventWithString:(CicadaEventWithString)eventWithString description:(NSString *)description {
    if (eventWithString == CICADA_EVENT_SWITCH_TO_SOFTWARE_DECODER) {
        [self.settingAndConfigView setIshardwareDecoder:NO];
    }else if (eventWithString == CICADA_EVENT_PLAYER_NETWORK_RETRY) {
        NSLog(@"network Retry");
                
        if (self.retryCount > 0) {
            if ((self.currentNetworkStatus == AFNetworkReachabilityStatusReachableViaWiFi) || (self.currentNetworkStatus == AFNetworkReachabilityStatusReachableViaWWAN && self.wanWillRetry)) {
                [self.player reload];
                self.retryCount --;
            }
        }else {
            [CicadaTool hudWithText:NSLocalizedString(@"重连失败" , nil) view:self.view];
        }
    }else if (eventWithString == CICADA_EVENT_PLAYER_NETWORK_RETRY_SUCCESS) {
        self.retryCount = 3;
    }
    [CicadaTool hudWithText:description view:self.view];
}

/**
 @brief 视频当前播放位置回调
 @param player 播放器player指针
 @param position 视频当前播放位置
 */
- (void)onCurrentPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    self.CicadaView.currentPosition = position;
}

/**
 @brief 视频缓存位置回调
 @param player 播放器player指针
 @param position 视频当前缓存位置
 */
- (void)onBufferedPositionUpdate:(CicadaPlayer*)player position:(int64_t)position {
    self.CicadaView.bufferPosition = position;
}

/**
 @brief 获取track信息回调
 @param player 播放器player指针
 @param info track流信息数组 参考CicadaTrackInfo
 */
- (void)onTrackReady:(CicadaPlayer*)player info:(NSArray<CicadaTrackInfo*>*)info {
    NSMutableArray * tracksArray = [NSMutableArray array];
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
    [tracksArray addObject:videoTracksArray];
    [tracksArray addObject:audioTracksArray];
    [tracksArray addObject:subtitleTracksArray];
    [self.settingAndConfigView setDataAndReloadWithArray:tracksArray];
    
    NSMutableArray *selectedArray = [NSMutableArray array];
    for (NSInteger i = 0; i<4; i++) {
        CicadaTrackInfo *eveinfo = [player getCurrentTrack:i];
        if (eveinfo) {
            [selectedArray addObject:[CicadaTool stringFromInt:eveinfo.trackIndex]];
        }
    }
    [self.settingAndConfigView setSelectedDataAndReloadWithArray:selectedArray];
}

- (void)onSubtitleExtAdded:(CicadaPlayer*)player trackIndex:(int)trackIndex URL:(NSString *)URL {
    NSLog(@"onSubtitleExtAdded: %@", URL);
    NSString *URLkey = @"";
    for (NSString *key in self.subtitleDictionary.allKeys) {
        if ([self.subtitleDictionary[key] isEqualToString:URL]) {
            URLkey = key;
            break;
        }
    }
    [CicadaTool hudWithText:[NSString stringWithFormat:@"%@%@%@",NSLocalizedString(@"外挂字幕" , nil),URLkey,NSLocalizedString(@"添加成功" , nil)] view:self.view];
    [self.settingAndConfigView.subtitleIndexDictionary setObject:[NSString stringWithFormat:@"%d",trackIndex] forKey:URLkey];
    [self.settingAndConfigView reloadTableView];
}

- (void)onSubtitleShow:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID subtitle:(NSString *)subtitle {
    [self.CicadaView setSubtitleAndShow:[CicadaTool filterHTML:subtitle]];
}

- (void)onSubtitleHide:(CicadaPlayer*)player trackIndex:(int)trackIndex subtitleID:(long)subtitleID {
    self.CicadaView.subTitleLabel.hidden = YES;
}

/**
 @brief 获取截图回调
 @param player 播放器player指针
 @param image 图像
 */
- (void)onCaptureScreen:(CicadaPlayer *)player image:(UIImage *)image {
    [CicadaShowImageView showWithImage:image inView:self.view];
}

/**
 @brief track切换完成回调
 @param player 播放器player指针
 @param info 切换后的信息 参考CicadaTrackInfo
 */
- (void)onTrackChanged:(CicadaPlayer*)player info:(CicadaTrackInfo*)info {
    NSString *description = nil;
    switch (info.trackType) {
        case CICADA_TRACK_VIDEO:
            description = [NSString stringWithFormat:@"%d", info.trackBitrate];
            break;
        case CICADA_TRACK_AUDIO:
            description = (nil != info.description)? info.description : info.audioLanguage;
            break;
        case CICADA_TRACK_SUBTITLE:
            description = (nil != info.description)? info.description : info.subtitleLanguage;
            break;
        default:
            break;
    }
    if (info.trackType == CICADA_TRACK_VIDEO) {
        [self.settingAndConfigView setCurrentVideo:description];
    }
    NSString *hudText = [description stringByAppendingString:NSLocalizedString(@"切换成功" , nil)];
    [CicadaTool hudWithText:hudText view:self.view];
}

/**
 @brief 获取缩略图成功回调
 @param positionMs 指定的缩略图位置
 @param fromPos 此缩略图的开始位置
 @param toPos 此缩略图的结束位置
 @param image 缩图略图像指针,对于mac是NSImage，iOS平台是UIImage指针
 */
- (void)onGetThumbnailSuc:(int64_t)positionMs fromPos:(int64_t)fromPos toPos:(int64_t)toPos image:(id)image {
    [self.CicadaView showThumbnailViewWithImage:(UIImage *)image];
}

/**
 @brief 获取缩略图失败回调
 @param positionMs 指定的缩略图位置
 */
- (void)onGetThumbnailFailed:(int64_t)positionMs {
    NSLog(@"获取缩略图失败");
}

/**
 @brief 视频缓冲进度回调
 @param player 播放器player指针
 @param progress 缓存进度0-100
 */
- (void)onLoadingProgress:(CicadaPlayer*)player progress:(float)progress {
    [self.CicadaView setLoadingViewProgress:(int)progress];
}

- (BOOL)setActive:(BOOL)active error:(NSError **)outError
{
    [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
    return [[AVAudioSession sharedInstance] setActive:active error:outError];
}

- (BOOL)setCategory:(NSString *)category withOptions:(AVAudioSessionCategoryOptions)options error:(NSError **)outError
{
//    self.enableMix = YES;
    if (self.enableMix) {
        options = AVAudioSessionCategoryOptionMixWithOthers | AVAudioSessionCategoryOptionDuckOthers;
    }
    return [[AVAudioSession sharedInstance] setCategory:category withOptions:options error:outError];
}

- (BOOL)setCategory:(AVAudioSessionCategory)category mode:(AVAudioSessionMode)mode routeSharingPolicy:(AVAudioSessionRouteSharingPolicy)policy options:(AVAudioSessionCategoryOptions)options error:(NSError **)outError
{
    if (self.enableMix) {
        return YES;
    }

    if (@available(iOS 11.0, tvOS 11.0, *)) {
        return [[AVAudioSession sharedInstance] setCategory:category mode:mode routeSharingPolicy:policy options:options error:outError];
    }
    return NO;
}

- (BOOL)onVideoPixelBuffer:(CVPixelBufferRef)pixelBuffer pts:(int64_t)pts
{
    NSLog(@"receive HW frame:%p pts:%lld", pixelBuffer, pts);
    return NO;
}

- (BOOL)onVideoRawBuffer:(uint8_t **)buffer lineSize:(int32_t *)lineSize pts:(int64_t)pts width:(int32_t)width height:(int32_t)height
{
    NSLog(@"receive SW frame:%p pts:%lld line0:%d line1:%d line2:%d width:%d, height:%d", buffer, pts,
          lineSize[0], lineSize[1], lineSize[2], width, height);
    return NO;
}

@end
