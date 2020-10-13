//
// Created by pingkai on 2020/10/9.
//

#include "AFAudioQueueRender.h"

#define LOG_TAG "AFAudioQueueRender"

#include <base/media/AVAFPacket.h>
#include <utils/ffmpeg_utils.h>
#include <utils/frame_work_log.h>
#include <utils/timer.h>

// TODO: let queue have a fixed size

using namespace Cicada;

AFAudioQueueRender AFAudioQueueRender::se(1);

void AFAudioQueueRender::OutputCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    auto *pThis = (AFAudioQueueRender *) inUserData;

    assert(pThis);
    if (!pThis->mRunning) return;
    if (pThis->mNeedFlush) {
        while (pThis->mInPut.size() > 0) {
            delete pThis->mInPut.front();
            pThis->mInPut.pop();
        }
        pThis->mNeedFlush = false;
    }
    if (pThis->mInPut.size() > 0) {
        copyPCMData(getAVFrame(pThis->mInPut.front()), (uint8_t *) inBuffer->mAudioData);
        pThis->mQueuedSamples += pThis->mInPut.front()->getInfo().audio.nb_samples;
        inBuffer->mAudioDataByteSize = static_cast<UInt32>(getPCMFrameLen(getAVFrame(pThis->mInPut.front())));
        delete pThis->mInPut.front();
        pThis->mInPut.pop();
    } else {
        memset(inBuffer->mAudioData, 0, inBuffer->mAudioDataByteSize);
        //    inBuffer->mAudioDataByteSize = 0;
    }

    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
}

AFAudioQueueRender::AFAudioQueueRender()
{
#if TARGET_OS_IPHONE
    AFAudioSessionWrapper::addObserver(this);
#endif
}

AFAudioQueueRender::AFAudioQueueRender(int dummy)
{
#if TARGET_OS_IPHONE
    AFAudioSessionWrapper::init();
#endif
    addPrototype(this);
}

AFAudioQueueRender::~AFAudioQueueRender()
{
#if TARGET_OS_IPHONE
    AFAudioSessionWrapper::removeObserver(this);
#endif
    mRunning = false;
    if (_audioQueueRef) {
        AudioQueueStop(_audioQueueRef, true);
        AudioQueueDispose(_audioQueueRef, true);
    }
    while (mInPut.size() > 0) {
        delete mInPut.front();
        mInPut.pop();
    }
}

void AFAudioQueueRender::fillAudioFormat()
{
    int bytesPerSample = 2;

    switch (mOutputInfo.format) {
        case AF_SAMPLE_FMT_S16:
            bytesPerSample = 2;
            mAudioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
            break;

        case AF_SAMPLE_FMT_S16P:
            bytesPerSample = 2;
            mAudioFormat.mFormatFlags = kAudioFormatFlagIsSignedInteger;
            break;

        case AF_SAMPLE_FMT_FLT:
            bytesPerSample = 4;
            mAudioFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
            break;

        case AF_SAMPLE_FMT_FLTP:
            bytesPerSample = 4;
            mAudioFormat.mFormatFlags = kAudioFormatFlagIsFloat;
            break;

        default:
            assert(0);
            break;
    }

    mAudioFormat.mSampleRate = mOutputInfo.sample_rate;
    mAudioFormat.mChannelsPerFrame = static_cast<UInt32>(mOutputInfo.channels);
    mAudioFormat.mBytesPerFrame = static_cast<UInt32>(mOutputInfo.channels * bytesPerSample);
    mAudioFormat.mFramesPerPacket = 1;
    mAudioFormat.mBitsPerChannel = static_cast<UInt32>(bytesPerSample * 8);
    mAudioFormat.mFormatID = kAudioFormatLinearPCM;
    mAudioFormat.mBytesPerPacket = mAudioFormat.mBytesPerFrame * mAudioFormat.mFramesPerPacket;
    mAudioFormat.mReserved = 0;
}

int AFAudioQueueRender::setSpeed(float speed)
{
    //  return filterAudioRender::setSpeed(speed);
    UInt32 timePitchBypass = 0;
    if (fabsf(speed - 1.0f) <= 0.000001) {
        timePitchBypass = 1;
        speed = 1.0f;
    }
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &timePitchBypass, sizeof(timePitchBypass));
    AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_PlayRate, speed);
    return 0;
}

int AFAudioQueueRender::init_device()
{
    fillAudioFormat();
    OSStatus status = AudioQueueNewOutput(&mAudioFormat, OutputCallback, this, nullptr, kCFRunLoopCommonModes, 0, &_audioQueueRef);
    if (status != noErr) {
        AF_LOGE("AudioQueue: AudioQueueNewOutput failed (%d)\n", (int) status);
        return status;
    }
    UInt32 propValue = 1;
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_EnableTimePitch, &propValue, sizeof(propValue));
    propValue = 1;
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
    propValue = kAudioQueueTimePitchAlgorithm_TimeDomain;
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchAlgorithm, &propValue, sizeof(propValue));
    return 0;
}

int AFAudioQueueRender::pause_device()
{
    mPlaying = false;
    if (_audioQueueRef) {
        AudioQueuePause(_audioQueueRef);
    }
    return 0;
}

int AFAudioQueueRender::start_device()
{
    mPlaying = true;
    if (_audioQueueRef) {
        OSStatus status = AudioQueueStart(_audioQueueRef, nullptr);
        if (status != noErr) {
            AF_LOGE("AudioQueue: AudioQueueStart failed (%d)\n", (int) status);
        }
    }
    return 0;
}

void AFAudioQueueRender::flush_device()
{
    if (mPlaying && _audioQueueRef) {
        AudioQueueFlush(_audioQueueRef);
    } else {
        for (auto item : _audioQueueBufferRefArray) {
            if (item && item->mAudioData) {
                memset(item->mAudioData, 0, item->mAudioDataByteSize);
            }
        }
    }
    mNeedFlush = true;
    mQueuedSamples = 0;
}

void AFAudioQueueRender::device_setVolume(float gain)
{
    float aq_volume = gain;
    if (fabsf(aq_volume - 1.0f) <= 0.000001) {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, 1.f);
    } else {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, aq_volume);
    }
}

int64_t AFAudioQueueRender::device_get_position()
{
    return static_cast<int64_t>(mQueuedSamples / (mAudioFormat.mSampleRate / 1000000));
}

int AFAudioQueueRender::device_write(unique_ptr<IAFFrame> &frame)
{
    if (mBufferAllocatedCount < QUEUE_SIZE) {
        int nb_samples = frame->getInfo().audio.nb_samples;
        UInt32 size = frame->getInfo().audio.nb_samples * mAudioFormat.mBytesPerFrame;
        AudioQueueAllocateBuffer(_audioQueueRef, size, &_audioQueueBufferRefArray[mBufferAllocatedCount]);
        copyPCMData(getAVFrame(frame.get()), (uint8_t *) _audioQueueBufferRefArray[mBufferAllocatedCount]->mAudioData);
        _audioQueueBufferRefArray[mBufferAllocatedCount]->mAudioDataByteSize = static_cast<UInt32>(size);
        AudioQueueEnqueueBuffer(_audioQueueRef, _audioQueueBufferRefArray[mBufferAllocatedCount], 0, nullptr);
        mQueuedSamples += nb_samples;
        mBufferAllocatedCount++;
        frame = nullptr;
        return 0;
    }
    if (mInPut.write_available() > 0) {
        mInPut.push(frame.release());
        return 0;
    }
    return -EAGAIN;
}

uint64_t AFAudioQueueRender::device_get_que_duration()
{
    if (mInPut.empty()) {
        return 0;
    }
    return mInPut.size() * mInPut.front()->getInfo().duration;
}

#if TARGET_OS_IPHONE

void AFAudioQueueRender::onInterrupted(Cicada::AF_AUDIO_SESSION_STATUS status)
{
    bool deal = false;

    if (status != AFAudioSessionMediaServicesWereReset && mListener) {
        if (mListener->onInterrupt(status == AFAudioSessionBeginInterruption)) {
            return;
        }
    }
    switch (status) {
        case AFAudioSessionEndInterruption:
            if (mPlaying) {
                AFAudioSessionWrapper::activeAudio();
                if (_audioQueueRef) {
                    AudioQueueStart(_audioQueueRef, nullptr);
                }
            }
            break;

        case AFAudioSessionBeginInterruption:
            if (mPlaying) {
                if (_audioQueueRef) {
                    AudioQueuePause(_audioQueueRef);
                }
            }
            break;

        case AFAudioSessionMediaServicesWereReset:
            assert(0);
            //            closeDevice();
            //            start_device();
            break;
        default:
            break;
    }
}
#endif
