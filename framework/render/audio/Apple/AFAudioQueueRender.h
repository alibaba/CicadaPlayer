//
// Created by pingkai on 2020/10/9.
//

#ifndef CICADAMEDIA_AFAUDIOQUEUERENDER_H
#define CICADAMEDIA_AFAUDIOQUEUERENDER_H

#include "../filterAudioRender.h"
#include <AudioToolbox/AudioToolbox.h>
#include <base/media/spsc_queue.h>
#include <render/audio/audioRenderPrototype.h>

#define QUEUE_SIZE (3)

#if TARGET_OS_IPHONE

#include "AFAudioSessionWrapper.h"

#endif
namespace Cicada {
    class AFAudioQueueRender : public filterAudioRender,
                               private audioRenderPrototype
#if TARGET_OS_IPHONE
        ,
                               public AFAudioSessionWrapper::listener
#endif

    {
    public:
        AFAudioQueueRender();

        ~AFAudioQueueRender() override;

        int setSpeed(float speed) override;

    private:
        int init_device() override;

        int pause_device() override;

        int start_device() override;

        void flush_device() override;

        void device_setVolume(float gain) override;

        int64_t device_get_position() override;

        int device_write(unique_ptr<IAFFrame> &frame) override;

        uint64_t device_get_que_duration() override;

    private:
        explicit AFAudioQueueRender(int dummy);

        Cicada::IAudioRender *clone() override
        {
            return new AFAudioQueueRender();
        }

        bool is_supported(AFCodecID codec) override
        {
            return true;
        };

        static AFAudioQueueRender se;

    private:
        void fillAudioFormat();

        static void OutputCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer);

#if TARGET_OS_IPHONE
        void onInterrupted(Cicada::AF_AUDIO_SESSION_STATUS status) override;

#endif
    private:
        AudioQueueBufferRef _audioQueueBufferRefArray[QUEUE_SIZE]{};
        AudioQueueRef _audioQueueRef{nullptr};
        AudioStreamBasicDescription mAudioFormat{};
        SpscQueue<IAFFrame *> mInPut{10};
        uint64_t mQueuedSamples{0};
        uint8_t mBufferAllocatedCount{0};
        bool mNeedFlush{false};
        bool mRunning{true};
        bool mPlaying{false};
        OSStatus mStartStatus{AVAudioSessionErrorCodeNone};
    };
}// namespace Cicada


#endif//CICADAMEDIA_AFAUDIOQUEUERENDER_H
