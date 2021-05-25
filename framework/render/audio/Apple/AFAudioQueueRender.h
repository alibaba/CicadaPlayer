//
// Created by pingkai on 2020/10/9.
//

#ifndef CICADAMEDIA_AFAUDIOQUEUERENDER_H
#define CICADAMEDIA_AFAUDIOQUEUERENDER_H

#include "../filterAudioRender.h"
#include <AudioToolbox/AudioToolbox.h>
#include <base/media/spsc_queue.h>
#include <render/audio/audioRenderPrototype.h>
#include <utils/afThread.h>

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

    private:
        bool device_require_format(const IAFFrame::audioInfo &info) override;

        int init_device() override;

        int pause_device() override;

        int start_device() override;

        void flush_device() override;

        void device_setVolume(float gain) override;

        int device_setSpeed(float speed) override;

        int64_t device_get_position() override;

        int device_write(unique_ptr<IAFFrame> &frame) override;

        uint64_t device_get_que_duration() override;

        void device_mute(bool bMute) override;

        uint64_t device_get_ability() override
        {
            return (uint64_t) A_FILTER_FLAG_TEMPO | A_FILTER_FLAG_VOLUME;
        }

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

        UInt32 copyAudioData(const AudioQueueBuffer *inBuffer, bool CopyFull);

        int audioQueueLoop();

        void flushAudioQueue();

#if TARGET_OS_IPHONE
        void onInterrupted(Cicada::AF_AUDIO_SESSION_STATUS status) override;

#endif
    private:
#define MAX_QUEUE_SIZE 4
        AudioQueueBufferRef _audioQueueBufferRefArray[MAX_QUEUE_SIZE]{};
        AudioQueueRef _audioQueueRef{nullptr};
        AudioStreamBasicDescription mAudioFormat{};
        SpscQueue<IAFFrame *> mInPut{10};
        int64_t mPlayedBufferSize{0};
        uint8_t mBufferAllocatedCount{0};
        bool mNeedFlush{false};
        bool mRunning{false};
        bool mPlaying{false};
        OSStatus mStartStatus{AVAudioSessionErrorCodeNone};
        unsigned int mReadOffset{0};
        UInt32 mAudioDataByteSize{0};
        uint8_t mBufferCount{0};
        std::unique_ptr<afThread> mAudioQueueThread{nullptr};
        uint8_t mInitStatus{0};
        float mQueueSpeed{1.0};
        float mVolume{1.0};
        bool mMute{false};
        std::atomic<uint64_t> mQueueDuration{0};
    };
}// namespace Cicada


#endif//CICADAMEDIA_AFAUDIOQUEUERENDER_H
