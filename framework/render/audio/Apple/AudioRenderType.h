//
//  AudioRenderType.h
//  audio
//
//  Created by huang_jiafa on 2019/01/18.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#ifndef AudioRenderType_H
#define AudioRenderType_H

namespace Cicada{
    typedef enum AF_AUDIO_SESSION_STATUS {
        AFAudioSessionBeginInterruption  = 1,
        AFAudioSessionEndInterruption    = 0,
        AFAudioSessionMediaServicesWereReset = 2,
    } AF_AUDIO_SESSION_STATUS;
}

#endif
