//
// Created by lifujun on 2019/8/29.
//

#ifndef CICADA_PLAYER_AUDOUNITRENDER_H
#define CICADA_PLAYER_AUDOUNITRENDER_H

#include <render/audio/filterAudioRender.h>
#include <MacTypes.h>
#include <AudioUnit/AudioUnit.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <boost/lockfree/spsc_queue.hpp>

#include "render/audio/audioRenderPrototype.h"
#include "AFAudioSessionWrapper.h"
#include <AudioToolbox/AUGraph.h>

#if TARGET_OS_OSX
#define AF_USE_MIX 0
#else
#define AF_USE_MIX 1
#endif

namespace Cicada {
    class AFAudioUnitRender : public filterAudioRender,
                              private audioRenderPrototype,
                              private AFAudioSessionWrapper::listener {
    public:
        AFAudioUnitRender();

        ~AFAudioUnitRender() override;

        virtual int onInterruption(AF_AUDIO_SESSION_STATUS status);

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
        void onInterrupted(AF_AUDIO_SESSION_STATUS status) override
        {
            onInterruption(status);
        }

    private:
        explicit AFAudioUnitRender(int dummy);

        Cicada::IAudioRender *clone() override
        {
            return new AFAudioUnitRender();
        }

        bool is_supported(AFCodecID codec) override
        {
            return true;
        };

        static AFAudioUnitRender se;

    private:

        void adjustOutputInfo();

        void fillAudioFormat();

        void changeAudioSample(void *pBuffer, int size);

    private:
        static bool haveError(OSStatus error, const char *operation);

        int loopChecker() override;

        void stopInUnit();

        int startInside();

        bool initDeviceInside();

        bool closeDevice();

        static OSStatus OnRenderCallback(void *inRefCon,
                                         AudioUnitRenderActionFlags *ioActionFlags,
                                         const AudioTimeStamp *inTimeStamp,
                                         UInt32 inOutputBusNumber,
                                         UInt32 inNumberFrames,
                                         AudioBufferList *ioData);

        OSStatus renderCallback(AudioUnitRenderActionFlags *ioActionFlags,
                                const AudioTimeStamp *inTimeStamp,
                                UInt32 inOutputBusNumber,
                                UInt32 inNumberFrames,
                                AudioBufferList *ioData);

    private:
#if AF_USE_MIX
        AUGraph mProcessingGraph = nullptr;
        AudioUnit mMixerUnit = nullptr;
#endif
        AudioUnit mAudioUnit = nullptr;

        std::atomic_bool mInit{false};
        std::atomic_bool mRunning{false};
        std::atomic_bool mNeedRerun{false};
        std::atomic_bool mDeviceWorking{false};
        // need waiting for input date became FULL
        std::atomic_bool mNeedWaitDateFull{false};
        bool mIsDummy = false;

#define QUEUE_CAPACITY 12
        typedef boost::lockfree::spsc_queue<IAFFrame *, boost::lockfree::capacity<QUEUE_CAPACITY>> audioQueue;
        audioQueue mAudioDataList{};
        unsigned int mReadOffset = 0;
        std::atomic_int64_t mTotalPlayedBytes{0};
        int mLastFetchSize = 0;
        int mMaxListSize = 4;
        std::atomic_uint64_t mPacketDuration{0};

        std::recursive_mutex mUnitMutex;
        std::mutex mFormatMutex;

        AudioStreamBasicDescription mAudioFormat;

        atomic<float> mVolume{1.0f};
#ifdef DUMP_AUDIO_DATA
        FILE *mFile = nullptr;
#endif
    };

}
#endif //CICADA_PLAYER_AUDOUNITRENDER_H
