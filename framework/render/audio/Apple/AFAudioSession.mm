//
//  AFAudioSession.mm
//  audio
//
//  Created by huang_jiafa on 2019/01/18.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#import <AudioToolbox/AudioToolbox.h>
#import <AVFoundation/AVFoundation.h>
#import <UIKit/UIKit.h>
#import "AFAudioSession.h"
#import "utils/frame_work_log.h"

@implementation AFAudioSession

- (id) init
{
    return [self init:nil];
}

- (id) init:(std::function<void(AF_AUDIO_SESSION_STATUS)>)func
{
    self = [super init];

    if (nil != self) {
        mFun = func;

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
    UInt8 reasonValue = [[notification.userInfo valueForKey:AVAudioSessionRouteChangeReasonKey] intValue];
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
    UInt8 theInterruptionType = [[notification.userInfo valueForKey:AVAudioSessionInterruptionTypeKey] intValue];

    AF_LOGI("Session %s\n", theInterruptionType == AVAudioSessionInterruptionTypeBegan ? "Begin Interruption" : "End Interruption");

    AF_AUDIO_SESSION_STATUS nID = AFAudioSessionEndInterruption;

    if (theInterruptionType == AVAudioSessionInterruptionTypeBegan) {
        nID = AFAudioSessionBeginInterruption;
    }
    else if (theInterruptionType == AVAudioSessionInterruptionTypeEnded) {
        //[[AVAudioSession sharedInstance] setActive:YES error:nil];
        nID = AFAudioSessionEndInterruption;
    }

    if (NULL != mFun) {
        mFun(nID);
    }
}

-(void) handleMediaServicesWereReset:(NSNotification *)notification
{
    AF_LOGI("Session handleMediaServicesWereReset");
    if (NULL != mFun) {
        mFun(AFAudioSessionMediaServicesWereReset);
    }
}

@end


