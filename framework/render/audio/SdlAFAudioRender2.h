//
// Created by yuyuan on 2020/12/01.
//

#ifndef FRAMEWORK_SDLAFAUDIORENDER2_H
#define FRAMEWORK_SDLAFAUDIORENDER2_H

#include "filterAudioRender.h"
#include <SDL2/SDL.h>

using namespace std;

namespace Cicada {
    class SdlAFAudioRender2 : public filterAudioRender {
    public:
        SdlAFAudioRender2();
        ~SdlAFAudioRender2() override;

    private:
        int init_device() override;
        int pause_device() override;
        int start_device() override;
        void flush_device() override;
        void device_setVolume(float gain) override;
        int64_t device_get_position() override;
        int device_write(unique_ptr<IAFFrame> &frame) override;
        uint64_t device_get_que_duration() override;
        int loopChecker() override;
        void device_preClose() override;
        uint64_t device_get_ability() override;

    private:
        bool mSdlAudioInited = false;
        SDL_AudioDeviceID mDevID{0};
        uint64_t mPlayedDuration = 0;
        uint8_t *mPcmBuffer = nullptr;
        int mPcmBufferSize = 0;
        uint8_t *mMixedBuffer = nullptr;
        int mMixedBufferSize = 0;
        std::atomic_int mOutputVolume{0};
        SDL_AudioSpec mSpec{0};
    };
}// namespace Cicada


#endif//FRAMEWORK_SDLAFAUDIORENDER2_H
