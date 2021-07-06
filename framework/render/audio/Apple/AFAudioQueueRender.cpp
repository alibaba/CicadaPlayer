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

AFAudioQueueRender AFAudioQueueRender::se(1);

//static int64_t gtime = INT64_MIN;

void AFAudioQueueRender::OutputCallback(void *inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
{
    auto *pThis = (AFAudioQueueRender *) inUserData;

    //        if (gtime != INT64_MIN) {
    //            AF_LOGD("time delta is %lld\n",af_gettime_ms() - gtime);
    //        }
    //        gtime =af_gettime_ms();

    UInt32 size = inBuffer->mAudioDataByteSize;
    assert(pThis);
    if (!pThis->mRunning) {
        return;
    }
    pThis->mPlayedBufferSize += inBuffer->mAudioDataByteSize;

    UInt32 miniSize = pThis->getMiniBufferSize(inBuffer->mAudioDataBytesCapacity);
    if (miniSize > 0) {
        size = miniSize;
    }

    inBuffer->mAudioDataByteSize = pThis->copyAudioData(inBuffer, size);
    if (inBuffer->mAudioDataByteSize == 0) {
        //    AF_LOGW("no audio data\n");
        inBuffer->mAudioDataByteSize = size;

        if (!pThis->mNeedFlush) {
            pThis->mPlayedBufferSize -= inBuffer->mAudioDataByteSize;
        }
        memset((uint8_t *) inBuffer->mAudioData, 0, inBuffer->mAudioDataByteSize);
    }
    AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, nullptr);
}

UInt32 AFAudioQueueRender::getMiniBufferSize(UInt32 maxBufferSize)
{
#if TARGET_OS_IPHONE
    if (IOSNotificationManager::Instance()->GetActiveStatus() == 0) {
        /*
	    *  MUST copy buffer fully, otherwise lots of bug will come
	    */
        return maxBufferSize;
    }
#endif

    if (mInPut.empty()) {
        return 0;
    }

    AVFrame *audioFrame = getAVFrame(mInPut.front());
    int frameBufferSize = getPCMFrameLen((const AVFrame *) audioFrame);
    int frameDuration = getPCMFrameDuration((const AVFrame *) audioFrame);
    int frameNum = 1;

    //at least 20ms
    int miniDuration = static_cast<int>(20000 * mQueueSpeed);
    if (frameDuration < miniDuration) {
        frameNum = miniDuration / frameDuration + 1;
    }

    UInt32 miniBufferSize = static_cast<UInt32>(frameBufferSize * frameNum);
    miniBufferSize = std::min(maxBufferSize, miniBufferSize);
    return miniBufferSize;
}

UInt32 AFAudioQueueRender::copyAudioData(const AudioQueueBuffer *inBuffer, UInt32 targetSize)
{
    if (!mRunning) {
        return 0;
    }
    UInt32 copySize = 0;
    int retryCont = 0;
    while (copySize < targetSize) {
        if (mInPut.size() > 0) {
            bool frameClear = false;
            size_t len = copyPCMDataWithOffset(getAVFrame(mInPut.front()), mReadOffset, (uint8_t *) inBuffer->mAudioData + copySize,
                                               targetSize - copySize, &frameClear);
            assert(len > 0);
            mReadOffset += len;
            copySize += len;
            if (frameClear) {
                if (mListener && !mNeedFlush) {
                    mListener->onFrameInfoUpdate(mInPut.front()->getInfo(), true);
                }
                
                bool rendered = false;
                if(mRenderingCb) {
                    rendered = mRenderingCb(mRenderingCbUserData , mInPut.front());
                }
                
                delete mInPut.front();
                mInPut.pop();
                mReadOffset = 0;
            }
        } else {
            if (retryCont++ < 0 && mRunning && !mNeedFlush) {
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
    mAudioQueueThread = unique_ptr<afThread>(NEW_AF_THREAD(audioQueueLoop));
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
    if (mAudioQueueThread) {
        mAudioQueueThread->stop();
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


    // the min size is 84ms
    mAudioDataByteSize = (mAudioFormat.mBytesPerFrame * mAudioFormat.mSampleRate) / 10;
}

int AFAudioQueueRender::device_setSpeed(float speed)
{
    //  return filterAudioRender::setSpeed(speed);
    UInt32 timePitchBypass = 0;
    if (fabsf(speed - 1.0f) <= 0.000001) {
        timePitchBypass = 1;
        speed = 1.0f;
    }
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &timePitchBypass, sizeof(timePitchBypass));
    AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_PlayRate, speed);
    mQueueSpeed = speed;
    return 0;
}

bool AFAudioQueueRender::device_require_format(const IAFFrame::audioInfo &info)
{
    // TODO: drop this
    if (info.format >= AF_SAMPLE_FMT_U8P) {
        return false;
    }
    mOutputInfo = info;
    return true;
}

int AFAudioQueueRender::init_device()
{
    fillAudioFormat();
    mRunning = true;
    mAudioQueueThread->start();
    while (mInitStatus == 0 && mRunning) {
        af_msleep(3);
    }
    if (mInitStatus < 0) {
        return -EINVAL;
    }
    return 0;
}

int AFAudioQueueRender::pause_device()
{
    if (mPlaying) {
        mPlaying = false;
        if (_audioQueueRef) {
            AudioQueuePause(_audioQueueRef);
        }
    }
    return 0;
}

int AFAudioQueueRender::start_device()
{
    if (!mPlaying) {
        mPlaying = true;
        if (_audioQueueRef) {
            mStartStatus = AudioQueueStart(_audioQueueRef, nullptr);
            if (mStartStatus != AVAudioSessionErrorCodeNone) {
                AF_LOGE("AudioQueue: AudioQueueStart failed (%d)\n", (int) mStartStatus);
            }
        }
    }
    return 0;
}

void AFAudioQueueRender::flush_device()
{
    mNeedFlush = true;
}
void AFAudioQueueRender::flushAudioQueue()
{ /*
 * fix me:
 * pause to avoid noise no flush, but it will spend too long time, so only do this when not mute
 * , it will speed up player stop
 */
    AudioQueueParameterValue volume = 0;
    if (_audioQueueRef) {
        AudioQueueGetParameter(_audioQueueRef, kAudioQueueParam_Volume, &volume);
    }
    if (mPlaying && volume > 0) {
        AudioQueuePause(_audioQueueRef);
    }
    for (auto item : _audioQueueBufferRefArray) {
        if (item && item->mAudioData) {
            memset(item->mAudioData, 0, item->mAudioDataByteSize);
        }
    }
    while (mInPut.size() > 0) {
        delete mInPut.front();
        mInPut.pop();
    }
    mReadOffset = 0;
    mPlayedBufferSize = 0;
    if (mPlaying) {
        mStartStatus = AudioQueueStart(_audioQueueRef, nullptr);
    }
}

void AFAudioQueueRender::device_setVolume(float gain)
{
    mVolume = gain;
    if (mMute) {
        return;
    }
    if (fabsf(gain - 1.0f) <= 0.000001) {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, 1.f);
    } else {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, gain);
    }
}

int64_t AFAudioQueueRender::device_get_position()
{
    if (mNeedFlush) {
        return 0;
    }
    return static_cast<int64_t>(mPlayedBufferSize / (mAudioFormat.mBytesPerFrame * (mAudioFormat.mSampleRate / 1000000)));
}

int AFAudioQueueRender::device_write(unique_ptr<IAFFrame> &frame)
{
    if (mNeedFlush || mInPut.write_available() <= 0) {
        return -EAGAIN;
    }
    if (mStartStatus != AVAudioSessionErrorCodeNone && mPlaying) {
        AF_LOGW("try start AudioQueue in error status %d\n", mStartStatus);
        if (_audioQueueRef) {
            mStartStatus = AudioQueueStart(_audioQueueRef, nullptr);
        }
    }
    if (mBufferCount == 0) {
        //FIXME: for low latency stream, queue another more buffer
        AVFrame *audioFrame = getAVFrame(frame.get());
        int frameDuration = getPCMFrameDuration((const AVFrame *) audioFrame);
        if (frameDuration < 20000) {
            mBufferCount = 4;
        } else {
            mBufferCount = 3;
        }
    }
    mInPut.push(frame.release());
    return 0;
}

uint64_t AFAudioQueueRender::device_get_que_duration()
{
    return mQueueDuration;
}
int AFAudioQueueRender::audioQueueLoop()
{
    OSStatus status =
            AudioQueueNewOutput(&mAudioFormat, OutputCallback, this, CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &_audioQueueRef);
    if (status != noErr) {
        AF_LOGE("AudioQueue: AudioQueueNewOutput failed (%d)\n", (int) status);
        mInitStatus = -1;
        return -1;
    }
    UInt32 propValue = 1;
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_EnableTimePitch, &propValue, sizeof(propValue));
    propValue = 1;
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchBypass, &propValue, sizeof(propValue));
    propValue = kAudioQueueTimePitchAlgorithm_TimeDomain;
    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_TimePitchAlgorithm, &propValue, sizeof(propValue));
    AudioChannelLayout layout;
    memset(&layout, 0, sizeof(AudioChannelLayout));

    switch (mAudioFormat.mChannelsPerFrame) {
        case 1:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Mono;
            break;
        case 2:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
            break;
        case 3:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_DVD_4;
            break;
        case 4:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Quadraphonic;
            break;
        case 5:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_5_0_A;
            break;
        case 6:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_5_1_A;
            break;
        case 7:
            /* FIXME: Need to move channel[4] (BC) to channel[6] */
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_6_1_A;
            break;
        case 8:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_7_1_A;
            break;
        default:
            layout.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
            break;
    }

    AudioQueueSetProperty(_audioQueueRef, kAudioQueueProperty_ChannelLayout, &layout, sizeof(AudioChannelLayout));
    mInitStatus = 1;

    while (mRunning) {
        if (mNeedFlush) {
            flushAudioQueue();
            mNeedFlush = false;
        }

        if (mBufferAllocatedCount < mBufferCount && mInPut.size() >= mBufferCount) {
            assert(mAudioDataByteSize > 0);
            assert(mBufferCount <= MAX_QUEUE_SIZE);
            while (mBufferAllocatedCount < mBufferCount && mRunning) {
                AudioQueueBuffer *buffer = nullptr;
                OSStatus err = AudioQueueAllocateBuffer(_audioQueueRef, mAudioDataByteSize, &buffer);
                if (err != noErr) {
                    AF_LOGE("AudioQueueAllocateBuffer error %d \n", err);
                    break;
                }
                _audioQueueBufferRefArray[mBufferAllocatedCount] = buffer;
                UInt32 miniBufferSize = getMiniBufferSize(buffer->mAudioDataBytesCapacity);
                buffer->mAudioDataByteSize = copyAudioData(buffer, miniBufferSize);
                assert(buffer->mAudioDataByteSize > 0);
                err = AudioQueueEnqueueBuffer(_audioQueueRef, buffer, 0, nullptr);
                if (err != noErr) {
                    AF_LOGE("AudioQueueEnqueueBuffer error %d \n", err);
                    assert(0);
                }
                mBufferAllocatedCount++;
            }
        }
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.2, 1);
        if (mInPut.empty())
            mQueueDuration = 0;
        else
            mQueueDuration = mInPut.size() * mInPut.front()->getInfo().duration;
    }
    if (_audioQueueRef) {
        AudioQueueStop(_audioQueueRef, true);
        AudioQueueDispose(_audioQueueRef, true);
    }
    return -1;
}
void AFAudioQueueRender::device_mute(bool bMute)
{
    mMute = bMute;
    if (bMute) {
        AudioQueueSetParameter(_audioQueueRef, kAudioQueueParam_Volume, 0);
    } else {
        device_setVolume(mVolume);
    }
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
