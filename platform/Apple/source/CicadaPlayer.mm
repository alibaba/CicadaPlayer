//
//  CicadaPlayer.mm
//  CicadaPlayer
//
//  Created by shiping.csp on 2018/11/16.
//  Copyright © 2018年 com.alibaba.AliyunPlayer. All rights reserved.
//

#include <TargetConditionals.h>

#import <list>

#import "MediaPlayer.h"
#import "CicadaPlayer.h"
#import "CicadaPlayerView.h"
#import "utils/af_string.h"
#import "utils/CicadaDynamicLoader.h"
#import "utils/frame_work_log.h"
#import "thumbnail/CicadaThumbnail.h"
#import "CicadaOCHelper.h"
#import "AFAudioSession.h"
#import "CicadaRenderCBWrapper.h"

using namespace std;
using namespace Cicada;

typedef void(^AliyunLogCallbackBlock) (CicadaLogLevel logLevel,NSString *strLog);
static AliyunLogCallbackBlock g_logBlock = nil;

//void playerLogSetLevel(int level, int enableConsole)
extern "C"
{

    void log_set_enable_console(int enable);

    void log_set_level(int level, int enable_console);

    void log_set_back(log_back func, void *arg);
};

static void CicadaPlayer_log_print(void *userData, int prio, const char *buf)
{
    if(g_logBlock){
        @autoreleasepool {
            NSString* strLog = [[NSString alloc] initWithUTF8String:buf];
            dispatch_queue_t queue =  dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);
            dispatch_async(queue, ^{
                g_logBlock((CicadaLogLevel)prio,strLog);
            });
        }
    }
}

typedef int64_t(^CicadaReferClockFun) ();

@interface CicadaPlayer () <CicadaPlayerViewDelegate>
{
    CicadaPlayerView* mView;
    CicadaConfig* mConfig;
    CicadaMediaInfo* mMediaInfo;
    CicadaOCHelper* mHelper;
}

#if ENABLE_CONAN
@property(nonatomic, strong) CicadaPlayerConan* conan;
#endif
@property(nonatomic, assign) MediaPlayer* player;
@property(nonatomic,assign) CicadaStatus mCurrentStatus;
@property(nonatomic, strong) NSString *traceId;
@property(nonatomic, strong) CicadaThumbnail* thumbnail;
@property(nonatomic, strong) CicadaReferClockFun referClock;
@end

@implementation CicadaPlayer
@synthesize autoPlay;
@synthesize width = _width;
@synthesize height = _height;
@synthesize currentPosition = _currentPosition;
@synthesize bufferedPosition = _bufferedPosition;
@synthesize duration = _duration;
@synthesize referClock = _referClock;

- (void)resetProperty
{
    _width = 0;
    _height = 0;
    _currentPosition = 0;
    _bufferedPosition = 0;
    _duration = -1;
}

- (instancetype)init
{
    return [self init:nil];
}

static int logOutput = 1;
+(void) setLogCallbackInfo:(CicadaLogLevel)logLevel callbackBlock:(void (^)(CicadaLogLevel logLevel,NSString* strLog))block;
{
    static dispatch_once_t disOnce;
    dispatch_once(&disOnce,^ {
        log_set_back(CicadaPlayer_log_print, nullptr);
    });

    log_set_level(logLevel, logOutput);
    g_logBlock = block;
}

- (instancetype)init:(NSString*)traceID
{
    if (self = [super init]) {
        self.traceId = traceID;
        self.player = new MediaPlayer();
        [self resetProperty];
        playerListener listener = {0};
        mHelper = new CicadaOCHelper(self);
        mHelper->getListener(listener);
        self.player->SetListener(listener);
        
        self.mCurrentStatus = CicadaStatusIdle;
        mView = nil;
        self.playerView = nil;
        
        mConfig = [[CicadaConfig alloc] init];
        mMediaInfo = [[CicadaMediaInfo alloc] init];

#if TARGET_OS_IPHONE
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(becomeActive)
                                                     name:UIApplicationDidBecomeActiveNotification
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(resignActive)
                                                     name:UIApplicationWillResignActiveNotification
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(willTerminate)
                                                     name:UIApplicationWillTerminateNotification
                                                   object:nil];

        if (UIApplicationStateActive != [[UIApplication sharedApplication] applicationState]) {
            self.player->EnterBackGround(true);
        }
#endif // TARGET_OS_IPHONE
    }
    return self;
}

-(void)dealloc
{
    [self destroy];
    
#if TARGET_OS_IPHONE
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidBecomeActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationWillResignActiveNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationWillTerminateNotification object:nil];
#endif // TARGET_OS_IPHONE
}

-(void) setConfig:(CicadaConfig*)config
{
    if (self.player && config) {
        Cicada::MediaPlayerConfig alivcConfig;
        alivcConfig.maxDelayTime = config.maxDelayTime;
        alivcConfig.maxBufferDuration = config.maxBufferDuration;
        alivcConfig.networkTimeout = config.networkTimeout;
        alivcConfig.highBufferDuration = config.highBufferDuration;
        alivcConfig.startBufferDuration = config.startBufferDuration;
        alivcConfig.networkRetryCount = config.networkRetryCount;
        
        if (nil != config.httpProxy) {
            alivcConfig.httpProxy = [config.httpProxy UTF8String];
        }
        if (nil != config.referer) {
            alivcConfig.referer = [config.referer UTF8String];
        }
        alivcConfig.bClearShowWhenStop = config.clearShowWhenStop==YES?true:false;
        if (nil != config.userAgent) {
            alivcConfig.userAgent = [config.userAgent UTF8String];
        }
        
        for (NSString* va in config.httpHeaders) {
            const char* header = [va UTF8String];
            alivcConfig.customHeaders.push_back(header);
        }
        
        self.player->SetConfig(&alivcConfig);
    }
}

-(BOOL) setCacheConfig:(CicadaCacheConfig *)cacheConfig
{
    if (self.player && cacheConfig) {
        CacheConfig config;
        config.mEnable = (cacheConfig.enable == YES);
        config.mMaxDirSizeMB = cacheConfig.maxSizeMB;
        config.mMaxDurationS = cacheConfig.maxDuration;
        if (nil != cacheConfig.path) {
            config.mCacheDir = [cacheConfig.path UTF8String];
        }

        self.player->SetCacheConfig(config);
    }

    return YES;
}

-(CicadaConfig*) getConfig
{
    if (self.player) {
        const Cicada::MediaPlayerConfig* config = self.player->GetConfig();
        if(config){
            mConfig.referer = [[NSString alloc] initWithUTF8String:config->referer.c_str()];
            mConfig.httpProxy = [[NSString alloc] initWithUTF8String:config->httpProxy.c_str()];
            mConfig.userAgent = [[NSString alloc] initWithUTF8String:config->userAgent.c_str()];
            mConfig.maxBufferDuration = config->maxBufferDuration;
            mConfig.maxDelayTime = config->maxDelayTime;
            mConfig.highBufferDuration = config->highBufferDuration;
            mConfig.startBufferDuration = config->startBufferDuration;
            mConfig.networkTimeout = config->networkTimeout;
            mConfig.clearShowWhenStop = config->bClearShowWhenStop;
            mConfig.networkRetryCount = config->networkRetryCount;
            
            [mConfig.httpHeaders removeAllObjects];
            for (int i=0;i<config->customHeaders.size();i++) {
                [mConfig.httpHeaders addObject:[[NSString alloc] initWithUTF8String:config->customHeaders[i].c_str()]];
            }
        }
    }
    
    return mConfig;
}

- (void)willTerminate
{
    [self stop];
    [self destroy];
}

- (void)becomeActive
{
    if (self.player) {
        self.player->EnterBackGround(false);
    }
}

- (void)resignActive
{
    if (self.player) {
        self.player->EnterBackGround(true);
    }
}

-(void)selectTrack:(int)trackIndex
{
    if (self.player) {
        self.player->SelectTrack(trackIndex);
    }
}

-(void) setThumbnailUrl:(NSString *)url {
    if (nil != url) {
        self.thumbnail = [[CicadaThumbnail alloc] init:[NSURL URLWithString:url]];
        self.thumbnail.delegate = self.delegate;
    } else {
        self.thumbnail = nil;
    }
}

-(CicadaMediaInfo*) getMediaInfo
{
    return mMediaInfo;
}

-(CicadaTrackInfo*) getCurrentTrack:(CicadaTrackType)type
{
    if (self.player) {
        StreamInfo* info = self.player->GetCurrentStreamInfo(static_cast<StreamType>(type));
        return CicadaOCHelper::getCicadaTrackInfo(info);
    }
    
    return nil;
}

- (void)notifyResize
{
    if (self.player == NULL)
        return;

//    self.player->Redraw();
}

#if TARGET_OS_OSX
-(void)setPlayerView:(NSView *)showView
{
    if (_playerView == showView) {
        return;
    }

    _playerView = showView;

    if (showView != nil) {
        if (mView == nil) {
            mView = [[CicadaPlayerView alloc] initWithFrame:showView.bounds];
            mView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
            [showView addSubview:mView];
            mView.delegate = self;
            mView.wantsBestResolutionOpenGLSurface = YES;

            if (self.player) {
                self.player->SetView((__bridge void*)mView);
            }
        }
        else {
            [mView removeFromSuperview];
            [mView setFrame:showView.bounds];
            [showView addSubview:mView];
        }
    }
}

#elif TARGET_OS_IPHONE
-(void)setPlayerView:(UIView *)showView
{
    if (self.player == NULL)
        return;
    
    if (showView != nil) {
        //TODO: 重新设置视图？
        if(mView == nil) {
            mView = [[CicadaPlayerView alloc] initWithFrame:showView.bounds];
            mView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            mView.delegate = self;

            [showView addSubview:mView];
            [showView bringSubviewToFront:mView];
            if(self.player){
                self.player->SetView((__bridge void*)mView.layer);
            }
        }
        else {
            [mView removeFromSuperview];
            [mView setFrame:showView.bounds];
            [showView addSubview:mView];
            [showView bringSubviewToFront:mView];
        }
        _playerView = showView;
    }
}
#endif // TARGET_OS_OSX

- (void)prepare
{
#if ENABLE_CONAN
    if (self.conan) {
        [self.conan start];
    }
#endif

    if (self.player) {
        self.player->Prepare();
    }
}

- (void)start
{
    if (self.player) {
        self.player->Start();
    }
}

- (void)pause
{
    if (self.player) {
        self.player->Pause();
    }
}

-(void)redraw
{
    if (self.player) {
//        self.player->Redraw();
    }
}

-(void) reset
{
    if (self.player) {
        [self stop];
        if (mView) {
            mView.delegate = nil;
        }

        if (self.player){
            self.player->SetView(nil);
        }
        
        [mView removeFromSuperview];
        mView = nil;
        _playerView = nil;
        mConfig = [[CicadaConfig alloc] init];
        mMediaInfo = [[CicadaMediaInfo alloc] init];
    }
}

- (void)stop
{
    if (self.player) {
        self.player->Stop();
        
        if (self.delegate && [self.delegate respondsToSelector:@selector(onPlayerStatusChanged:oldStatus:newStatus:)]) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self.delegate onPlayerStatusChanged:self oldStatus:self.mCurrentStatus newStatus:CicadaStatusStopped];
                self.mCurrentStatus = CicadaStatusStopped;
            });
        }
    }
    [self resetProperty];
}

- (void) setRenderDelegate:(id<CicadaRenderDelegate>)theDelegate
{
    _renderDelegate = theDelegate;
    if (self.player) {
        self.player->SetOnRenderFrameCallback(CicadaRenderCBWrapper::OnRenderFrame, (__bridge void*)theDelegate);
    }
}

- (void)setInnerDelegate:(id<CicadaDelegate>) delegate
{
    mHelper->setDelegate(delegate);
}

-(void)destroy
{
    if (mView) {
        mView.delegate = nil;
    }

    if (self.player) {
        delete self.player;
        self.player = nullptr;
    }

    if (mHelper) {
        delete mHelper;
        mHelper = nullptr;
    }
    
    if (mView) {
        [mView removeFromSuperview];
        mView = nil;
    }

#if ENABLE_CONAN
//    NSLog(@"conan destroy start");
    if (self.conan) {
        [self.conan destroy];
        self.conan = nil;
    }
//    NSLog(@"conan destroy end");
#endif
}

-(void)seekToTime:(int64_t)time seekMode:(CicadaSeekMode)seekMode
{
    if (self.player) {
        SeekMode mode = SEEK_MODE_ACCURATE;
        switch (seekMode) {
            case CICADA_SEEKMODE_ACCURATE:
                mode = SEEK_MODE_ACCURATE;
                break;
            case CICADA_SEEKMODE_INACCURATE:
                mode = SEEK_MODE_INACCURATE;
                break;
            default:
                break;
        }
        self.player->SeekTo(time, mode);
    }
}

-(void)setMaxAccurateSeekDelta:(int)delta
{
    if (self.player) {
        self.player->SetOption("maxAccurateSeekDelta", AfString::to_string(delta).c_str());
    }
}

-(void) setDuration:(int64_t)value
{
    if (self.player) {
        _duration = self.player->GetDuration();
    }
}

-(int64_t) getDuration
{
    if ((-1 == _duration) && self.player) {
        _duration = self.player->GetDuration();
    }
    return _duration;
}

// for kvo, use internal
-(void)setCurrentPosition:(int64_t)value
{
    _currentPosition = value;
}

-(void)setBufferedPosition:(int64_t)value
{
    _bufferedPosition = value;
}
    
-(void)setWidth:(int)value
{
    _width = value;
}

-(void)setHeight:(int)value
{
    _height = value;
}

//update in PositionUpdate
//-(int64_t)getCurrentPosition
//{
//    if (self.player) {
//        _currentPosition = self.player->GetCurrentPosition();
//    }
//    return _currentPosition;
//}
//-(int64_t)getBufferedPosition
//{
//    if (self.player) {
//        _bufferedPosition = self.player->GetBufferedPosition();
//    }
//    return _bufferedPosition;
//}

-(int)rotation
{
    if (self.player) {
        return self.player->GetVideoRotation();
    }
    return 0;
}

-(void)setMuted:(BOOL)muted
{
    if (self.player) {
        self.player->SetMute(muted==YES?true:false);
    }
}

-(BOOL)isMuted
{
    if (self.player) {
        return self.player->IsMuted();
    }

    return NO;
}

-(void)setVolume:(float)_volume
{
    if (self.player) {
        self.player->SetVolume(_volume);
    }
}

-(float)volume
{
    if (self.player) {
        return self.player->GetVolume();
    }
    
    return 0;
}

//void ApsaraLogCallbackF(void *userData, int prio, char *buf)
//{
//    NSLog(@"ApsaraLogCallback:%s", buf);
//}

-(void)setEnableLog:(BOOL)enableLog
{
    log_set_enable_console(enableLog);
    _enableLog = enableLog;
    logOutput = enableLog;
    //IApsaraVideoPlayer::SetLogCallback(ApsaraLogCallbackF, nullptr);
}

-(BOOL)getEnableLog
{
    if (self.player) {
        return NO;
    }
    return _enableLog;
}

-(void)setAutoPlay:(BOOL)value
{
    autoPlay = value;
    if (self.player) {
        self.player->SetAutoPlay(value==YES?true:false);
    }
}

-(BOOL)isAutoPlay
{
    if (self.player) {
     //   autoPlay = self.player->IsAutoPlay()==true?YES:NO;
    }
    return autoPlay;
}

-(void)setEnableHardwareDecoder:(BOOL)value
{
    _enableHardwareDecoder = value;
    if (self.player) {
        self.player->EnableHardwareDecoder(value==YES?true:false);
    }
}

-(void)setRotateMode:(CicadaRotateMode)rotateMode
{
    if (self.player) {
        RotateMode rm = ROTATE_MODE_0;
        switch (rotateMode) {
            case CICADA_ROTATE_0:
                rm = ROTATE_MODE_0;
                break;
            case CICADA_ROTATE_90:
                rm = ROTATE_MODE_90;
                break;
            case CICADA_ROTATE_180:
                rm = ROTATE_MODE_180;
                break;
            case CICADA_ROTATE_270:
                rm = ROTATE_MODE_270;
                break;
            default:
                break;
        }
        self.player->SetRotateMode(rm);
    }
}

-(CicadaRotateMode)rotateMode
{
    CicadaRotateMode rotate = CICADA_ROTATE_0;
    if (self.player) {
        RotateMode rm = self.player->GetRotateMode();
        switch (rm) {
            case ROTATE_MODE_0:
                rotate = CICADA_ROTATE_0;
                break;
            case ROTATE_MODE_90:
                rotate = CICADA_ROTATE_90;
                break;
            case ROTATE_MODE_180:
                rotate = CICADA_ROTATE_180;
                break;
            case ROTATE_MODE_270:
                rotate = CICADA_ROTATE_270;
                break;
            default:
                break;
        }
    }
    return rotate;
}

-(void)setMirrorMode:(CicadaMirrorMode)mirrorMode
{
    if (self.player) {
        switch (mirrorMode) {
            case CICADA_MIRRORMODE_VERTICAL:
                self.player->SetMirrorMode(MIRROR_MODE_VERTICAL);
                break;
            case CICADA_MIRRORMODE_HORIZONTAL:
                self.player->SetMirrorMode(MIRROR_MODE_HORIZONTAL);
                break;
            case CICADA_MIRRORMODE_NONE:
                self.player->SetMirrorMode(MIRROR_MODE_NONE);
                break;
            default:
                break;
        }
    }
}

-(CicadaMirrorMode)mirrorMode
{
    CicadaMirrorMode mirror = CICADA_MIRRORMODE_NONE;
    if (self.player) {
        MirrorMode mm = self.player->GetMirrorMode();
        switch (mm) {
            case MIRROR_MODE_HORIZONTAL:
                mirror = CICADA_MIRRORMODE_HORIZONTAL;
                break;
            case MIRROR_MODE_VERTICAL:
                mirror = CICADA_MIRRORMODE_VERTICAL;
            default:
                break;
        }
    }
    return mirror;
}

-(void)setRate:(float)rate
{
    if (self.player) {
        self.player->SetSpeed(rate);
    }
}

-(float)rate
{
    if (self.player) {
        return self.player->GetSpeed();
    }
    
    return 1.0f;
}

-(void)setLoop:(BOOL)loop
{
    if (self.player) {
        self.player->SetLoop(loop==YES?true:false);
    }
}

-(BOOL)isLoop
{
    if (self.player) {
        return self.player->IsLoop();
    }
    return NO;
}

-(void)setScalingMode:(CicadaScalingMode)scalingMode
{
    if (self.player) {
        ScaleMode mode = ScaleMode::SM_FIT;
        switch (scalingMode) {
            case CICADA_SCALINGMODE_SCALETOFILL:
                mode = ScaleMode::SM_EXTRACTTOFIT;
                break;
            case CICADA_SCALINGMODE_SCALEASPECTFIT:
                mode = ScaleMode::SM_FIT;
                break;
            case CICADA_SCALINGMODE_SCALEASPECTFILL:
                mode = ScaleMode::SM_CROP;
                break;
            default:
                break;
        }
       
        self.player->SetScaleMode(mode);
    }
}

-(CicadaScalingMode)scalingMode
{
    if (self.player) {
        ScaleMode mode = (ScaleMode)(self.player->GetScaleMode());
        switch (mode) {
            case ScaleMode::SM_EXTRACTTOFIT:
                return CICADA_SCALINGMODE_SCALETOFILL;
            case ScaleMode::SM_FIT:
                return CICADA_SCALINGMODE_SCALEASPECTFIT;
            case ScaleMode::SM_CROP:
                return CICADA_SCALINGMODE_SCALEASPECTFILL;
            default:
                break;
        }
    }

    return CICADA_SCALINGMODE_SCALEASPECTFIT;
}

-(void)getThumbnail:(int64_t)positionMs
{
    if (self.thumbnail) {
        [self.thumbnail getThumbnail:positionMs];
    }
}

-(void) snapShot
{
    if (self.player) {
        self.player->CaptureScreen();
    }
}

-(void)setUrlSource:(CicadaUrlSource *)source
{
    if (self.player && source) {
        NSURL* url = source.playerUrl;
        if (url) {
            NSString* chUrl = [url absoluteString];
            chUrl = [chUrl stringByRemovingPercentEncoding];
            self.player->SetDataSource([chUrl UTF8String]);
        }
    }
}

-(NSString *) getCacheFilePath:(NSString *)URL
{
    if (self.player && (nil != URL)) {
        string ret = self.player->GetCachePathByURL([URL UTF8String]);
        return [NSString stringWithUTF8String:ret.c_str()];
    }
    return nil;
}

-(void) addExtSubtitle:(NSString *)URL
{
    if (self.player && URL) {
        self.player->AddExtSubtitle([URL UTF8String]);
    }
}

-(void) selectExtSubtitle:(int)trackIndex enable:(BOOL)enable
{
    if (self.player) {
        self.player->SelectExtSubtitle(trackIndex, enable);
    }
}

-(void) reload
{
    if (self.player) {
        self.player->Reload();
    }
}

-(NSString *) getPropertyString:(CicadaPropertyKey)key
{
    if (self.player) {
        std::string str = self.player->GetPropertyString((PropertyKey)key);
        return [NSString stringWithUTF8String:str.c_str()];
    }
    return @"";
}

-(void) setDefaultBandWidth:(int)bandWidth
{
    if (self.player) {
        self.player->SetDefaultBandWidth(bandWidth);
    }
}

- (void) setDelegate:(id<CicadaDelegate>)theDelegate
{
    _delegate = theDelegate;
    if (self.player && [_delegate respondsToSelector:@selector(onVideoRendered:timeMs:pts:)]) {
        self.player->EnableVideoRenderedCallback(true);
    }
}

-(void) setPlaybackType:(CicadaPlaybackType)type
{
    if (self.player) {
        uint64_t flags = 0;
        switch (type) {
            case CicadaPlaybackTypeVideo:
                flags = (1 << CICADA_TRACK_VIDEO);
                break;
            case CicadaPlaybackTypeAudio:
                flags = (1 << CICADA_TRACK_AUDIO);
                break;
            default:
                flags = (1 << CICADA_TRACK_VIDEO) | (1 << CICADA_TRACK_AUDIO);
                break;
        }
        self.player->SetStreamTypeFlags(flags);
    }
}

-(int64_t) getPlayingPts
{
    if (self.player) {
        return self.player->GetMasterClockPts();
    }
    return 0;
}

int64_t CicadaClockRefer(void *arg)
{
    CicadaPlayer *player = (__bridge CicadaPlayer *)arg;
    if (player.referClock) {
        return player.referClock();
    }
    return -1;
}

-(void) SetClockRefer:(int64_t (^)(void))referClock
{
    if (self.player) {
        self.player->SetClockRefer(CicadaClockRefer, (__bridge void*)self);
    }
    self.referClock = referClock;
}

-(NSString *) getOption:(CicadaOption)key
{
    if (nullptr == self.player) {
        return @"";
    }

    char value[256] = {0};

    switch (key) {
        case CICADA_OPTION_RENDER_FPS:
            self.player->GetOption("renderFps", value);
            break;
        default:
            break;
    }

    return [NSString stringWithUTF8String:value];
}

+ (NSString *) getSDKVersion
{
    string version = MediaPlayer::GetSdkVersion();
    return [[NSString alloc] initWithUTF8String:version.c_str()];
}

+ (void) initPlayerComponent:(NSString *)functionName function:(void*)function
{
    if (nil != functionName && 0 < [functionName length] && nullptr != function) {
        CicadaDynamicLoader::addFunctionToMap([functionName UTF8String], function);
    }
}

+ (void)setAudioSessionDelegate:(id<CicadaAudioSessionDelegate>)delegate
{
#if TARGET_OS_IPHONE
    [AFAudioSession sharedInstance].delegate = delegate;
#endif
}

@end
