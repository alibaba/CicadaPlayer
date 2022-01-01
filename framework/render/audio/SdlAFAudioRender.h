//
// Created by lifujun on 2019/7/23.
//

#ifndef FRAMEWORK_SDLAFAUDIORENDER_H
#define FRAMEWORK_SDLAFAUDIORENDER_H

#include "IAudioRender.h"
#include <SDL2/SDL.h>
#include <base/media/AVAFPacket.h>
#include <filter/ffmpegAudioFilter.h>
#include <list>
#include <mutex>
#include <queue>

using namespace std;

namespace Cicada {
    class SdlAFAudioRender : public IAudioRender {
    public:
        SdlAFAudioRender();

        ~SdlAFAudioRender() override;

        int init(const IAFFrame::audioInfo *info) override;

        int renderFrame(std::unique_ptr<IAFFrame> &frame, int timeOut) override;

        int renderFrame(std::unique_ptr<IAFPacket> &packet, int timeOut) override;

        int64_t getPosition() override;

        uint64_t getQueDuration() override;

        void mute(bool bMute) override;

        int setVolume(float volume) override;

        int setSpeed(float speed) override;

        void pause(bool bPause) override;

        void flush() override;

        void prePause() override;

    private:
        static void SDLAudioCallback(void *userdata, Uint8 *stream, int len);

    private:
        bool mInited = false;
        queue<unique_ptr<IAFFrame>> mAudioFrames;

        int mStartPos = 0;

        uint8_t *pcmBuffer = nullptr;
        int bufferSize = 0;

        SDL_AudioSpec mSpec{0};
        SDL_AudioDeviceID mDevID{};

        std::unique_ptr<Cicada::ffmpegAudioFilter> mFiler{};
        uint64_t mPlayedDuration = 0;
        IAFFrame::audioInfo mInfo{};
        float mSpeed{1};
        float mVolume = 1.0f;
        std::atomic<bool> mMute{false};
    };
}// namespace Cicada


#endif//FRAMEWORK_SDLAFAUDIORENDER_H
