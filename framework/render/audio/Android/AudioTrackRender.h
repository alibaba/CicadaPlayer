//
// Created by lifujun on 2019/8/29.
//

#ifndef SOURCE_AUDIOTRACKRENDER_H
#define SOURCE_AUDIOTRACKRENDER_H


#include <render/audio/filterAudioRender.h>
#include <jni.h>
#include "../audioRenderPrototype.h"
#include <atomic>

class AudioTrackRender :
        public Cicada::filterAudioRender,
        private audioRenderPrototype {
public:
    AudioTrackRender();

    ~AudioTrackRender();

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
    AudioTrackRender(int dummy)
    {
        addPrototype(this);
    }

    bool is_supported(AFCodecID codec) override
    {
        return true;
    }

    Cicada::IAudioRender *clone() override
    {
        return new AudioTrackRender();
    }

    static AudioTrackRender se;

private:

    void adjustOutputInfo();

    int init_jni();

    uint64_t getDevicePlayedSimples();

    void flush_device_inner() ;

    uint64_t getPlayedPosition();

private:

    jobject audio_track{nullptr};
    jmethodID method_write{nullptr};
    jmethodID method_play{nullptr};
    jmethodID method_pause{nullptr};
    jmethodID method_flush{nullptr};
    jmethodID method_stop{nullptr};
    jmethodID method_release{nullptr};
    jmethodID method_setVolume{nullptr};
    jmethodID method_getPlayState{nullptr};
    jmethodID method_getPosition{nullptr};


private:
    std::atomic<uint64_t> mBasePlayedPosition{0};
    uint8_t *mPcmBuffer = nullptr;
    int64_t mPcmBufferLen = 0;

    int64_t mPlayedBufferLen = 0;
    std::atomic<int64_t> mAudioFlushPosition{0};

    std::atomic<uint64_t> mSendSimples{0};
    int mSimpleSize{0};
    jobject jbuffer{nullptr};

    enum class FlushRestPosition{
        unknow , reset, notReset
    };

    std::atomic<FlushRestPosition> mFlushPositionReset{FlushRestPosition::unknow};

};


#endif //SOURCE_AUDIOTRACKRENDER_H
