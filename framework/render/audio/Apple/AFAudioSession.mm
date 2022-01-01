//
//  AFAudioSession.mm
//  audio
//
//  Created by huang_jiafa on 2019/01/18.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//
#define LOG_TAG "AFAudioSession"
#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import "AFAudioSession.h"
#import "utils/frame_work_log.h"
#import "codec/utils_ios.h"

@implementation AFAudioSession

+(instancetype) sharedInstance
{
    static AFAudioSession *instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[AFAudioSession alloc] init];
    });
    return instance;
}

- (instancetype) init
{
    self = [super init];

    if (nil != self) {
        mFun = nullptr;

        AVAudioSession *sessionInstance = [AVAudioSession sharedInstance];

        // add the interruption handler
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(handleInterruption:)
                                                     name:AVAudioSessionInterruptionNotification
                                                   object:sessionInstance];

        // we don't do anything special in the route change notification
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(handleRouteChange:)
                                                     name:AVAudioSessionRouteChangeNotification
                                                   object:sessionInstance];

        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(handleMediaServicesWereReset:)
                                                     name:AVAudioSessionMediaServicesWereResetNotification
                                                   object:sessionInstance];
    }

    return self;
}

-(void) setCallback:(std::function<void(AF_AUDIO_SESSION_STATUS)>)func
{
    mFun = func;
}

-(void) handleRouteChange:(NSNotification *)notification
{
    NSInteger reasonValue = [[notification.userInfo valueForKey:AVAudioSessionRouteChangeReasonKey] intValue];
    AVAudioSessionRouteDescription *routeDescription = [notification.userInfo valueForKey:AVAudioSessionRouteChangePreviousRouteKey];

    switch (reasonValue) {
        case AVAudioSessionRouteChangeReasonNewDeviceAvailable:
            AF_LOGI("NewDeviceAvailable");
            break;
        case AVAudioSessionRouteChangeReasonOldDeviceUnavailable:
            AF_LOGI("OldDeviceUnavailable");
            break;
        case AVAudioSessionRouteChangeReasonCategoryChange:
            AF_LOGI("CategoryChange");
            break;
        case AVAudioSessionRouteChangeReasonOverride:
            AF_LOGI("Override");
            break;
        case AVAudioSessionRouteChangeReasonWakeFromSleep:
            AF_LOGI("WakeFromSleep");
            break;
        case AVAudioSessionRouteChangeReasonNoSuitableRouteForCategory:
            AF_LOGI("NoSuitableRouteForCategory");
            break;
        default:
            AF_LOGI("ReasonUnknown");
    }

    NSString *str = [NSString stringWithFormat:@"%@", routeDescription];
    if (nil != str) {
        AF_LOGI("routeDescription: %s", [str UTF8String]);
    }
}

-(void) handleInterruption:(NSNotification *)notification
{
    NSDictionary *userInfo = notification.userInfo;
    if (!userInfo || !userInfo[AVAudioSessionInterruptionTypeKey]) {
        return;
    }
    NSInteger interruptionType = [notification.userInfo[AVAudioSessionInterruptionTypeKey] integerValue];
    NSInteger interruptionOption = [notification.userInfo[AVAudioSessionInterruptionOptionKey] integerValue];
    BOOL delayedSuspendedNotification = NO;
    AF_LOGD("interruptionType is %d, interruptionOption is %d\n", interruptionType, interruptionOption);

    if (@available(iOS 10.3, *)) {
        if ([notification.userInfo objectForKey:AVAudioSessionInterruptionWasSuspendedKey]) {
            delayedSuspendedNotification = (long) [notification.userInfo objectForKey:AVAudioSessionInterruptionWasSuspendedKey] == 1;
        }
    }
    AF_LOGD("delayedSuspendedNotification is %d\n", delayedSuspendedNotification);

    if (interruptionType == AVAudioSessionInterruptionTypeBegan && !delayedSuspendedNotification) {
        // Playback interrupted by an incoming phone call.
        if (mFun) {
            mFun(AFAudioSessionBeginInterruption);
        }
    }
    if (interruptionType == AVAudioSessionInterruptionTypeEnded && interruptionOption == AVAudioSessionInterruptionOptionShouldResume) {
        if (mFun) {
            mFun(AFAudioSessionEndInterruption);
        }
    }
}

-(void) handleMediaServicesWereReset:(NSNotification *)notification
{
    AF_LOGI("Session handleMediaServicesWereReset");
    if (NULL != mFun) {
        mFun(AFAudioSessionMediaServicesWereReset);
    }
}

-(int) activeAudio
{
    NSError *err = nil;
    bool active = IOSNotificationManager::Instance()->GetActiveStatus() != 0;
    AF_LOGI("setActive when app acitve is :%d", active);

    AVAudioSessionCategoryOptions options = 0;
    if (self.delegate && [self.delegate respondsToSelector:@selector(setCategory:withOptions:error:)]) {
        [self.delegate setCategory:AVAudioSessionCategoryPlayback withOptions:options error:&err];
    } else {
        [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback withOptions:options error:&err];
    }

    // Airplay 2 feature
    if (@available(iOS 11.0, tvOS 11.0, *)) {
        if (self.delegate && [self.delegate respondsToSelector:@selector(setCategory:mode:routeSharingPolicy:options:error:)]) {
            [self.delegate setCategory:AVAudioSessionCategoryPlayback mode:AVAudioSessionModeDefault routeSharingPolicy:AVAudioSessionRouteSharingPolicyLongForm options:options error:&err];
        } else {
            [[AVAudioSession sharedInstance] setCategory:AVAudioSessionCategoryPlayback mode:AVAudioSessionModeDefault routeSharingPolicy:AVAudioSessionRouteSharingPolicyLongForm options:options error:&err];
        }
    }

    if (self.delegate && [self.delegate respondsToSelector:@selector(setActive:error:)]) {
        if (![self.delegate setActive:YES error:&err]) {
            AF_LOGE("setActive error:%d", err.code);
        }
    } else {
        if (![[AVAudioSession sharedInstance] setActive:YES error:&err]) {
            AF_LOGE("setActive error:%d", err.code);
        }
    }

    return static_cast<int>(err.code);
}

@end


