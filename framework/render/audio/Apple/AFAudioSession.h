//
//  AFAudioSession.h
//  audio
//
//  Created by huang_jiafa on 2019/01/18.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "AudioRenderType.h"
#include <functional>

using namespace Cicada;

@interface AFAudioSession : NSObject
{
    std::function<void(AF_AUDIO_SESSION_STATUS)> mFun;
}

-(id) init:(std::function<void(AF_AUDIO_SESSION_STATUS)>)func;
-(void) setCallback:(std::function<void(AF_AUDIO_SESSION_STATUS)>)func;

@end
