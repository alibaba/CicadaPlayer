//
// Created by moqi on 2019/10/21.
//

#import <mutex>
#import <render/audio/IAudioRender.h>
#import <list>
#import "AFAudioSessionWrapper.h"
#import "AFAudioSession.h"
#import <AVFoundation/AVFoundation.h>
#import "codec/utils_ios.h"
#import <UIKit/UIKit.h>

AFAudioSession *gAFAudioSession =  [[AFAudioSession alloc] init];
std::mutex gAFRenderMutex;

std::list<AFAudioSessionWrapper::listener *> AFAudioSessionWrapper::mListenerList;

void AFAudioSessionWrapper::init()
{
  //  gAFAudioSession = [[AFAudioSession alloc] init];

    if (nil != gAFAudioSession) {
        std::function<void(AF_AUDIO_SESSION_STATUS)> fun = [](AF_AUDIO_SESSION_STATUS state) -> void {
            return onInterruption(state);
        };

        [gAFAudioSession setCallback:fun];
    }
}

void AFAudioSessionWrapper::addObserver(listener *li)
{
    mListenerList.push_back(li);
}

void AFAudioSessionWrapper::removeObserver(listener *li)
{
    mListenerList.remove(li);
}

void AFAudioSessionWrapper::onInterruption(AF_AUDIO_SESSION_STATUS status)
{
    for (auto &listener : mListenerList) {
        listener->onInterrupted(status);
    }
}

// call from main thread
int AFAudioSessionWrapper::activeAudio()
{
    NSError *err = nil;
    AVAudioSession *sessionInstance = [AVAudioSession sharedInstance];

    bool active = IOSNotificationManager::Instance()->GetActiveStatus() != 0;
    if (!active) {
        // need ReceivingRemoteControlEvents or use mix module to start in background
        // otherwise will lead to 560557684 error
        [[UIApplication sharedApplication] beginReceivingRemoteControlEvents];
//        [sessionInstance setCategory:AVAudioSessionCategoryPlayback
//                         withOptions:AVAudioSessionCategoryOptionMixWithOthers | AVAudioSessionCategoryOptionDuckOthers
//                              error:nil];
    }
    AF_LOGI("setActive when app acitve is :%d", active);
    [sessionInstance setCategory:AVAudioSessionCategoryPlayback withOptions:0 error:nil];
    // Airplay 2 feature
    if (@available(iOS 11.0, tvOS 11.0, *)) {
        AVAudioSessionCategoryOptions options = 0;
        [sessionInstance setCategory:AVAudioSessionCategoryPlayback mode:AVAudioSessionModeDefault routeSharingPolicy:AVAudioSessionRouteSharingPolicyLongForm options:options error:&err];
    }

    if (![sessionInstance setActive:YES error:&err]) {
        AF_LOGE("setActive error:%d", err.code);
    }

    return static_cast<int>(err.code);
}
