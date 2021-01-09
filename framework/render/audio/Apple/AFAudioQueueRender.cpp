//
// Created by pingkai on 2020/10/9.
//

#include "AFAudioQueueRender.h"

#define LOG_TAG "AFAudioQueueRender"

#include <codec/utils_ios.h>

#include <base/media/AVAFPacket.h>
#include <utils/ffmpeg_utils.h>
#include <utils/frame_work_log.h>
#include <utils/timer.h>

using namespace Cicada;

#define AUDIO_BUFFER_SIZE (32768ll)

AFAudioQueueRender AFAudioQueueRender::se(1);

void AFAudioQueueRender::OutputCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    auto *pThis = (AFAudioQueueRender *) inUserData;

    assert(pThis);
    if (!pThis->mRunning) {
        return;
    }
    pThis->mPlayedBufferSize += inBuffer->mAudioDataByteSize;
    inBuffer->mAudioDataByteSize = pThis->copyAudioData(inBuffer);
    if (inBuffer->mAudioDataByteSize == 0) {
        AF_LOGW("no audio data\n");
        inBuffer->mAudioDataByteSize = inBuffer->mAudioDataBytesCapacity;

        if (!pThis->mNeedFlush) {
            pThis->mPlayedBufferSize -= inBuffer->mAudioDataByteSize;
        }
        memset((uint8_t *) inBuffer->mAudioData, 0, inBuffer->mAudioDataByteSize);
    }
    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
}
UInt32 AFAudioQueueRender::copyAudioData(const AudioQueueBuffer *inBuffer)
{
    if (mNeedFlush) {
        while (mInPut.size() > 0) {
            delete mInPut.front();
            mInPut.pop();
        }
        mReadOffset = 0;
        mNeedFlush = false;
        return 0;
    }
    UInt32 copySize = 0;
    int retryCont = 0;
    while (copySize < inBuffer->mAudioDataBytesCapacity) {
        if (mInPut.size() > 0) {
            bool frameClear = false;
            size_t len = copyPCMDataWithOffset(getAVFrame(mInPut.front()), mReadOffset, (uint8_t *) inBuffer->mAudioData + copySize,
                                               inBuffer->mAudioDataBytesCapacity - copySize, &frameClear);
            assert(len > 0);
            mReadOffset += len;
            copySize += len;
            if (frameClear) {
                if (mListener) {
                    mListener->onUpdateTimePosition(mInPut.front()->getInfo().timePosition);
                }
                delete mInPut.front();
                mInPut.pop();
                mReadOffset = 0;
#if TARGET_OS_IPHONE
                if (IOSNotificationManager::Instance()->GetActiveStatus() != 0) {
                    break;
                }
#endif
            }
        } else {
            if (retryCont++ < 3 && mRunning && !mNeedFlush) {
                af_msleep(5);
            } else {
                break;
            }
        }
    }
    return copySize;
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
    /*
     * There is a performance problem when use copyPCMData to copy a planer audio,
     * use ffmpeg filter convert it to packed audio
     */
    if (mOutputInfo.format >= AF_SAMPLE_FMT_U8P) {
        mOutputInfo.format -= AF_SAMPLE_FMT_U8P;
        needFilter = true;
    }

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
        mStartStatus = AudioQueueStart(_audioQueueRef, nullptr);
        if (mStartStatus != AVAudioSessionErrorCodeNone) {
            AF_LOGE("AudioQueue: AudioQueueStart failed (%d)\n", (int) mStartStatus);
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
    mPlayedBufferSize = 0;
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
    return static_cast<int64_t>(mPlayedBufferSize / (mAudioFormat.mBytesPerFrame * (mAudioFormat.mSampleRate / 1000000)));
}

int AFAudioQueueRender::device_write(unique_ptr<IAFFrame> &frame)
{
    if (mNeedFlush || mInPut.write_available() <= 0) {
        return -EAGAIN;
    }
    mInPut.push(frame.release());
    if (mBufferAllocatedCount < QUEUE_SIZE) {
        AudioQueueBuffer *buffer = NULL;
        AudioQueueAllocateBuffer(_audioQueueRef, AUDIO_BUFFER_SIZE, &buffer);
        _audioQueueBufferRefArray[mBufferAllocatedCount] = buffer;
        buffer->mAudioDataByteSize = copyAudioData(buffer);
        AudioQueueEnqueueBuffer(_audioQueueRef, _audioQueueBufferRefArray[mBufferAllocatedCount], 0, nullptr);
        mBufferAllocatedCount++;
    }
    return 0;
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
        deal = mListener->onInterrupt(status == AFAudioSessionBeginInterruption);
    }
    if (status == AFAudioSessionEndInterruption && mStartStatus != AVAudioSessionErrorCodeNone) {
        /*
         Start audioQueue would fail during interrupted, restart it on interrupt end.
         */
        deal = false;
    }
    if (deal) {
        return;
    }
    switch (status) {
        case AFAudioSessionEndInterruption:
            if (mPlaying && _audioQueueRef) {
                AFAudioSessionWrapper::activeAudio();
                mStartStatus = AudioQueueStart(_audioQueueRef, nullptr);
                if (mStartStatus != AVAudioSessionErrorCodeNone) {
                    AF_LOGE("AudioQueueStart error at interrupt end %d\n", mStartStatus);
                }
            }
            break;

        case AFAudioSessionBeginInterruption:
            if (mPlaying && _audioQueueRef) {
                OSStatus status1 = AudioQueuePause(_audioQueueRef);
                if (status1 != AVAudioSessionErrorCodeNone) {
                    AF_LOGE("AudioQueuePause error at interrupt %d\n", status1);
                }
            }
            break;

        case AFAudioSessionMediaServicesWereReset:
            assert(0);
            AF_LOGE("AFAudioSessionMediaServicesWereReset\n");
            //            closeDevice();
            //            start_device();
            break;
        default:
            break;
    }
}
#endif
