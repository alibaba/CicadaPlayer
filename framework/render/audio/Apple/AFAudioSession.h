//
//  AFAudioSession.h
//  audio
//
//  Created by huang_jiafa on 2019/01/18.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AudioRenderType.h"
#import "CicadaAudioSessionDelegate.h"
#import <functional>

using namespace Cicada;

@interface AFAudioSession : NSObject
{
    std::function<void(AF_AUDIO_SESSION_STATUS)> mFun;
}

@property (nullable, nonatomic, strong) id <CicadaAudioSessionDelegate> delegate;

+(instancetype _Nonnull) sharedInstance;
-(void) setCallback:(std::function<void(AF_AUDIO_SESSION_STATUS)>)func;
-(int) activeAudio;

@end
