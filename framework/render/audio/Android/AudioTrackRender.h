//
// Created by lifujun on 2019/8/29.
//

#ifndef SOURCE_AUDIOTRACKRENDER_H
#define SOURCE_AUDIOTRACKRENDER_H


#include "../audioRenderPrototype.h"
#include <atomic>
#include <base/media/spsc_queue.h>
#include <jni.h>
#include <render/audio/filterAudioRender.h>
#include <utils/afThread.h>

class AudioTrackRender :
        public Cicada::filterAudioRender,
        private audioRenderPrototype {
public:
    AudioTrackRender();

    ~AudioTrackRender();

private:
    bool device_require_format(const IAFFrame::audioInfo &info) override;

    int init_device() override;

    int pause_device() override;

    int start_device() override;

    void flush_device() override;

    void device_setVolume(float gain) override;

    void device_mute(bool bMute) override;

    int64_t device_get_position() override;

    int device_write(unique_ptr<IAFFrame> &frame) override;

    uint64_t device_get_que_duration() override;

    uint64_t device_get_ability() override
    {
        return A_FILTER_FLAG_VOLUME;
    }

    void device_preClose() override
    {
        mRunning = false;
    }

private:
    AudioTrackRender(int dummy) : mFrameQueue(1)
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

    int device_write_internal(IAFFrame *frame);

    static AudioTrackRender se;

private:

    void adjustOutputInfo();

    int init_jni();

    uint64_t getDevicePlayedSimples();

    uint64_t getDeviceQuequedDuration();

    void flush_device_inner(bool clearFrameQueue);

    int write_loop();

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
    uint8_t *mPcmBuffer = nullptr;
    int64_t mPcmBufferLen = 0;

    int64_t mPlayedBufferLen = 0;
    std::atomic<uint64_t> mAudioFlushPosition{0};

    std::atomic<uint64_t> mSendSimples{0};
    int jBufferLen{0};
    jobject jbuffer{nullptr};

    Cicada::SpscQueue<IAFFrame *> mFrameQueue;
    afThread *mWriteThread{nullptr};
    std::atomic_bool mRunning{false};
    int mMaxQueSize{2};
    float mVolume{1.0};
    bool mMute{false};
    std::unique_ptr<IAFFrame::audioInfo> mRequireFormat{};
};


#endif //SOURCE_AUDIOTRACKRENDER_H
