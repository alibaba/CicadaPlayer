//
// Created by lifujun on 2019/8/29.
//

#define LOG_TAG "AFAudioUnitRender"

#include "AudioRenderType.h"
#include <AudioToolbox/AudioToolbox.h>
#include <utils/timer.h>
#include "AFAudioUnitRender.h"
#include <utils/ffmpeg_utils.h>
#include <base/media/AVAFPacket.h>

#if TARGET_OS_OSX
    #define MAX_RETRY 3
#else
    #define MAX_RETRY 5
#endif

#if AF_USE_MIX
    #define autoAudioUnit mMixerUnit
#else
    #define autoAudioUnit mAudioUnit
#endif

namespace Cicada {
    static const AFSampleFormat sampleFormats[] = {
        AF_SAMPLE_FMT_S16, AF_SAMPLE_FMT_S16P,
#if !AF_USE_MIX
        AF_SAMPLE_FMT_FLT, AF_SAMPLE_FMT_FLTP,
#endif
    };
    // TODO: support more format  s8 ...
    static const int sampleRates[] = {
        8000, 11025, 12000, 16000, 22050, 24000, 32000, 44100, 48000
    };
    AFAudioUnitRender AFAudioUnitRender::se(0);

    bool AFAudioUnitRender::haveError(OSStatus error, const char *operation)
    {
        if (error == noErr) {
            return false;
        }

        AF_LOGE("Error: %s", operation);
        return true;
    }

    AFAudioUnitRender::AFAudioUnitRender()
    {
        AF_LOGI("AFAudioUnitRender:%p", this);
        memset(&mAudioFormat, 0, sizeof(AudioStreamBasicDescription));
#if TARGET_OS_IPHONE
        AFAudioSessionWrapper::addObserver(this);
#endif
    }

    AFAudioUnitRender::~AFAudioUnitRender()
    {
        if (mIsDummy) {
            return;
        }

#if TARGET_OS_IPHONE
        AFAudioSessionWrapper::removeObserver(this);
#endif
        int64_t nTime = af_getsteady_ms();

        if (mRenderThread) {
            mRenderThread->stop();
        }

        closeDevice();
        flush_device();
        AF_LOGI("~AFAudioUnitRender :%p deconstruct use:%lld", this, af_getsteady_ms() - nTime);
#ifdef DUMP_AUDIO_DATA

        if (NULL != mFile) {
            fclose(mFile);
        }

#endif
    }

    int AFAudioUnitRender::loopChecker()
    {
        if (!mRunning) {
            startInside();
        }

        return 0;
    }

    int AFAudioUnitRender::init_device()
    {
        adjustOutputInfo();
        fillAudioFormat();
        bool ret = initDeviceInside();

        if (!ret) {
            return -1;
        }

        return ret;
    }

    void AFAudioUnitRender::adjustOutputInfo()
    {
        bool match = false;

        for (auto item : sampleFormats) {
            if (mInputInfo.format == item) {
                match = true;
                break;
            }
        }

        if (!match) {
            mOutputInfo.format = AF_SAMPLE_FMT_S16;
            needFilter = true;
        }

        int sampleRate = -1;

        for (auto &item : sampleRates) {
            if (mInputInfo.sample_rate <= item) {
                sampleRate = item;
                break;
            }
        }

        if (mInputInfo.sample_rate != sampleRate) {
            if (sampleRate > 0) {
                mOutputInfo.sample_rate = sampleRate;
            } else {
                int size = sizeof(sampleRates) / sizeof(sampleRates[0]);

                if (size > 0) {
                    mOutputInfo.sample_rate = sampleRates[size - 1];
                } else {
                    mOutputInfo.sample_rate = 48000;
                }
            }

            needFilter = true;
        }

        if (mInputInfo.channels > 2) {
            mOutputInfo.channels = 2;
            needFilter = true;
        }
    }

    bool AFAudioUnitRender::initDeviceInside()
    {
        std::unique_lock<std::recursive_mutex> locker(mUnitMutex);
        flush_device();
        OSErr iErrCode = noErr;

        if (!mInit) {
            closeDevice();
            // Describe the output unit.
            AudioComponentDescription ioDescription = {0};
            ioDescription.componentType = kAudioUnitType_Output;
#if TARGET_OS_IPHONE
            ioDescription.componentSubType = kAudioUnitSubType_RemoteIO;
#else
            ioDescription.componentSubType = kAudioUnitSubType_DefaultOutput;
#endif
            ioDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
#if AF_USE_MIX
            AF_LOGI("init audio mixer");

            if (haveError(NewAUGraph(&mProcessingGraph),
                          "NewAUGraph Error")) {
                return false;
            }

            AUNode iONode;

            if (haveError(AUGraphAddNode(mProcessingGraph, &ioDescription, &iONode),
                          "AUGraphAddNode I/O")) {
                return false;
            }

            // Describe the mixer node
            AudioComponentDescription mixerDescription = {0};
            mixerDescription.componentType = kAudioUnitType_Mixer;
            mixerDescription.componentSubType = kAudioUnitSubType_MultiChannelMixer;
            mixerDescription.componentManufacturer = kAudioUnitManufacturer_Apple;
            AUNode mixerNode;

            if (haveError(AUGraphAddNode(mProcessingGraph, &mixerDescription, &mixerNode),
                          "AUGraphAddNode mix")) {
                return false;
            }

            if (haveError(AUGraphConnectNodeInput(mProcessingGraph, mixerNode, 0, iONode, 0),
                          "AUGraphConnectNodeInput IONode")) {
                return false;
            }

            if (haveError(AUGraphOpen(mProcessingGraph),
                          "AUGraphConnectNode open")) {
                return false;
            }

            if (haveError(AUGraphNodeInfo(mProcessingGraph, iONode, NULL, &mAudioUnit),
                          "AUGraphNodeInfo I/O result")) {
                return false;
            }

            if (haveError(AUGraphNodeInfo(mProcessingGraph, mixerNode, NULL, &mMixerUnit),
                          "AUGraphNodeInfo Mixer")) {
                return false;
            }

            //setup mixer unit
            if (haveError(AudioUnitSetParameter(mMixerUnit,
                                                kMultiChannelMixerParam_Enable,
                                                kAudioUnitScope_Input,
                                                (unsigned int)0,
                                                1,
                                                0),
                          "Couldn't set the render callback on the audio unit")) {
                return false;
            }

#else
            AF_LOGI("init audio unit");
            // get output
            AudioComponent ac = AudioComponentFindNext(NULL, &ioDescription);

            if (NULL == ac) {
                AF_LOGE("AudioComponentFindNext failed");
                return false;
            }

            // Create a new unit based on this that we'll use for output
            iErrCode = AudioComponentInstanceNew(ac, &mAudioUnit);

            if (nil == mAudioUnit) {
                AF_LOGE("Error AudioComponentInstanceNew:%d", iErrCode);
                return false;
            }

#endif
            AURenderCallbackStruct callbackStruct;
            callbackStruct.inputProc = OnRenderCallback;
            callbackStruct.inputProcRefCon = this;
#if AF_USE_MIX

            if (haveError(AUGraphSetNodeInputCallback(mProcessingGraph, mixerNode, 0, &callbackStruct),
                          "Error set OnRenderCallback")) {
                return false;
            }

            // support lock screen
            UInt32 maxFPS = 4096;

            if (haveError(AudioUnitSetProperty(mMixerUnit, kAudioUnitProperty_MaximumFramesPerSlice,
                                               kAudioUnitScope_Global, 0, &maxFPS, sizeof(maxFPS)),
                          "Couldn't start the output unit")) {
                return false;
            }

            if (haveError(AudioUnitSetProperty(mAudioUnit, kAudioUnitProperty_MaximumFramesPerSlice,
                                               kAudioUnitScope_Global, 0, &maxFPS, sizeof(maxFPS)),
                          "Couldn't start the output unit")) {
                return false;
            }

            if (haveError(AUGraphInitialize(mProcessingGraph),
                          "Couldn't start the output unit")) {
                return false;
            }

#else

            if (haveError(AudioUnitSetProperty(autoAudioUnit,
                                               kAudioUnitProperty_SetRenderCallback,
                                               kAudioUnitScope_Input,
                                               0,
                                               &callbackStruct,
                                               sizeof(callbackStruct)),
                          "Error set OnRenderCallback")) {
                return false;
            }

            if (haveError(AudioUnitInitialize(mAudioUnit),
                          "Error AudioUnitInitialize")) {
                return false;
            }

#endif
        }

        iErrCode = AudioUnitSetProperty(autoAudioUnit,
                                        kAudioUnitProperty_StreamFormat,
                                        kAudioUnitScope_Input,
                                        0,
                                        &mAudioFormat,
                                        sizeof(AudioStreamBasicDescription));
        AF_LOGI("AFAudioUnitRender SetFormat mSampleRate:%e, mFormatID:%ld, mBytesPerPacket:%u, mBytesPerFrame:%u, mChannelsPerFrame:%u, mBitsPerChannel:%u, mFormatFlags:%d",
                mAudioFormat.mSampleRate, mAudioFormat.mFormatID, mAudioFormat.mBytesPerPacket,
                mAudioFormat.mBytesPerFrame, mAudioFormat.mChannelsPerFrame, mAudioFormat.mBitsPerChannel, mAudioFormat.mFormatFlags);

        if (noErr != iErrCode) {
            AF_LOGE("Error AudioUnitSetProperty:%d", iErrCode);
            return false;
        }

        mInit = true;
        return true;
    }

    int AFAudioUnitRender::pause_device()
    {
        std::unique_lock<std::recursive_mutex> locker(mUnitMutex);
        int64_t nTime = af_getsteady_ms();
#if AF_USE_MIX
        Boolean isRunning = false;
        OSStatus result = AUGraphIsRunning(mProcessingGraph, &isRunning);

        if (result) {
            printf("AUGraphIsRunning result %ld %08lX %4.4s\n", (long)result, (long)result, (char *)&result);
            return -1;
        }

        if (isRunning) {
            result = AUGraphStop(mProcessingGraph);

            if (result) {
                printf("AUGraphStop result %ld %08lX %4.4s\n", (long)result, (long)result, (char *)&result);
                return -1;
            }
        }

        mRunning = false;
        mDeviceWorking = false; // stop directly
#else
        mRunning = false;
//        stopInUnit();
#endif
        AF_LOGI("AFAudioUnitRender pause use:%lld", af_getsteady_ms() - nTime);
        return 0;
    }

    int AFAudioUnitRender::start_device()
    {
        std::unique_lock<std::recursive_mutex> locker(mUnitMutex);

        if (mRunning) {
            AF_LOGI("AFAudioUnitRender start_device in running");
            return 0;
        } else if (mDeviceWorking) {
            AF_LOGI("AFAudioUnitRender start_device in mDeviceWorking");
            mNeedWaitDateFull = true;
            mRunning = true;
        }

        // will start in looper checker
        return 0;
    }

    int AFAudioUnitRender::startInside()
    {
        if (!mInit) {
            if (!initDeviceInside()) {
                return -1;
            }
        }

        int64_t nTime = af_getsteady_ms();
        {
            std::unique_lock<std::recursive_mutex> locker(mUnitMutex);
            AF_LOGI("AFAudioUnitRender start");

            if (nil == mAudioUnit) {
                AF_LOGE("AudioUnit is NULL");
                return -1;
            }

            if (mRunning) {
                return 0;
            }

            mNeedWaitDateFull = true;
            mRunning = true;
        }
        {
            std::unique_lock<std::recursive_mutex> locker(mUnitMutex);

            if (mRunning && !mDeviceWorking) {
                AF_LOGI("Trying AudioOutputUnitStart");
#if AF_USE_MIX

                if (haveError(AUGraphStart(mProcessingGraph), "Couldn't start the output unit")) {
                    mRunning = false;
                    return -1;
                }

#else
                // Start playback
                OSErr iErrCode = AudioOutputUnitStart(mAudioUnit);

                if (noErr != iErrCode) {
                    mRunning = false;
                    AF_LOGE("Error AudioOutputUnitStart:%d", iErrCode);
                    return -1;
                }

#endif
                mDeviceWorking = true;
            }

            mNeedRerun = false;
            AF_LOGI("AFAudioUnitRender startup use:%lld", af_getsteady_ms() - nTime);
        }
        return 0;
    }

    void AFAudioUnitRender::flush_device()
    {
        bool isRun = mRunning;

        if (isRun) {
            pause_device();
        }

#if !AF_USE_MIX

        while (mDeviceWorking) {
            af_msleep(2);
        }

#endif

        // make sure the read thread be paused.
        while (!mAudioDataList.empty()) {
            delete mAudioDataList.front();
            mAudioDataList.pop();
        }

        mReadOffset = 0;
        mTotalPlayedBytes = 0;
        mNeedWaitDateFull = true;

        if (isRun) {
            start_device();
        }
    }

    void AFAudioUnitRender::device_setVolume(float gain)
    {
        mVolume = gain;
#if AF_USE_MIX
        //        AudioUnitSetParameter(mAudioUnit, kAUNBandEQParam_GlobalGain, kAudioUnitScope_Global, 0, mVolume, 0);
        AudioUnitSetParameter(mMixerUnit, kMultiChannelMixerParam_Volume, kAudioUnitScope_Output, 0, mVolume, 0);
#endif
    }

    int64_t AFAudioUnitRender::device_get_position()
    {
        return static_cast<int64_t>(mTotalPlayedBytes / mAudioFormat.mBytesPerFrame / (mAudioFormat.mSampleRate / 1000000.0f));
    }

    void AFAudioUnitRender::fillAudioFormat()
    {
        int bytesPerSample = 2;

        switch (mOutputInfo.format) {
            case AF_SAMPLE_FMT_S16:
                bytesPerSample = 2;
                mAudioFormat.mFormatFlags =
                    kAudioFormatFlagIsSignedInteger | kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked;
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

    bool AFAudioUnitRender::closeDevice()
    {
        bool bRet = true;
        mTotalPlayedBytes = 0;

        if (nil != mAudioUnit) {
            std::unique_lock<std::recursive_mutex> locker(mUnitMutex);
#if AF_USE_MIX
            pause_device();

            if (haveError(AUGraphUninitialize(mProcessingGraph), "Couldn't uninialize the output unit")) {
                return false;
            }

#else
            mRunning = false;
            int iMaxLoop = 10;

            while (mDeviceWorking) {
                if (0 > iMaxLoop--) {
                    break;
                }

                af_msleep(2);
            }

            // Call stop before AudioUnitUninitialize
            // since the stop in RenderProc not end immediate
            if (haveError(AudioOutputUnitStop(mAudioUnit), "close device AudioOutputUnitStop")) {
                return false;
            }

            // for audio unit need uninitialize
            if (haveError(AudioUnitUninitialize(mAudioUnit),
                          "close device AudioUnitUninitialize")) {
                return false;
            }

#endif
            mDeviceWorking = false;

            if (haveError(AudioComponentInstanceDispose(mAudioUnit),
                          "close device AudioComponentInstanceDispose")) {
                return false;
            }

            mAudioUnit = nil;
#if AF_USE_MIX

            if (haveError(AudioComponentInstanceDispose(mMixerUnit),
                          "Couldn't dispose the mixer unit")) {
                return false;
            }

            mMixerUnit = nil;
#endif
            mInit = false;
            AF_LOGI("close device success");
        }

        return bRet;
    }

    OSStatus AFAudioUnitRender::OnRenderCallback(void *inRefCon,
            AudioUnitRenderActionFlags *ioActionFlags,
            const AudioTimeStamp *inTimeStamp,
            UInt32 inOutputBusNumber,
            UInt32 inNumberFrames,
            AudioBufferList *ioData)
    {
        AFAudioUnitRender *render = (AFAudioUnitRender *) inRefCon;
        OSStatus status = render->renderCallback(ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
        render->changeAudioSample(ioData->mBuffers[0].mData, ioData->mBuffers[0].mDataByteSize);
        return status;
    }

    OSStatus AFAudioUnitRender::renderCallback(
        AudioUnitRenderActionFlags *ioActionFlags,
        const AudioTimeStamp *inTimeStamp,
        UInt32 inOutputBusNumber,
        UInt32 inNumberFrames,
        AudioBufferList *ioData)
    {
        if (!mRunning) {
            stopInUnit();
        }

        if (!mRunning || mNeedWaitDateFull) {
            // memset to avoid noise
            for (int i = 0; i < ioData->mNumberBuffers; ++i) {
                void *buffer = ioData->mBuffers[i].mData;
                memset(buffer, 0, ioData->mBuffers[i].mDataByteSize);
            }

            return noErr;
        }

        size_t i, remaining;
        AudioBuffer *abuf;
        uint8_t *ptr;

        if (mLastFetchSize < ioData->mBuffers[0].mDataByteSize) {
            mLastFetchSize = ioData->mBuffers[0].mDataByteSize;
        }

        for (i = 0; i < ioData->mNumberBuffers; i++) {
            abuf = &ioData->mBuffers[i];
            remaining = abuf->mDataByteSize;
            ptr = (uint8_t *) abuf->mData;
            int retry = 0;

            while (remaining > 0) {
                if (mAudioDataList.read_available() > 0) {
                    IAFFrame *frame = mAudioDataList.front();

                    if (frame) {
                        AVAFFrame *avafFrame = dynamic_cast<AVAFFrame *>(frame);
                        bool frameClear = false;
                        size_t len = copyPCMDataWithOffset(avafFrame->ToAVFrame(), mReadOffset, ptr, remaining, &frameClear);
                        remaining -= len;
                        ptr += len;
                        mReadOffset += len;
                        mTotalPlayedBytes += len;

                        if (frameClear) {
                            delete frame;
                            mAudioDataList.pop();
                            mReadOffset = 0;
                        }

                        // if success, continue
                        continue;
                    }
                }

                // wait less then buffer time won't lead to noise
                af_msleep(2);
                ++retry;

                if (retry > MAX_RETRY) {
                    AF_LOGD("renderCallback needWait :%p", this);
                    mNeedWaitDateFull = true;
                    memset(ptr, 0, remaining);
                    remaining = 0;
                }
            }
        }

        return noErr;
    }

#define CHANGE(type, step, bit) \
    int bit1 = bit - 1; \
    int nTmp; \
    type * pSData; \
    pSData = (type *)pBuffer; \
    for (int i = 0; i < size; i+=step) { \
        nTmp = (int)((*pSData) * mVolume); \
        if (nTmp >= -bit && nTmp <= bit1) { \
            *pSData = (short)nTmp;\
        } else if(nTmp < -bit) { \
            *pSData = -bit; \
        } else if(nTmp > bit1) { \
            *pSData = bit1; \
        } \
        pSData++; \
    }

    void AFAudioUnitRender::changeAudioSample(void *pBuffer, int size)
    {
#ifdef DUMP_AUDIO_DATA

        if (NULL == mFile) {
            static int input = 0;
            char *home = getenv("HOME");
            char filePath[256] = {0};
            sprintf(filePath, "%s/%s/%d_audio_%d.pcm", home, "Documents", input++, (int)mAudioFormat.mSampleRate);
            mFile = fopen(filePath, "wb");
        }

        if (NULL == mFile) {
            AF_LOGI("Open audio.pcm file error!");
            return;
        }

        if (size > 0) {
            fwrite(pBuffer, size, 1, mFile);
            fflush(mFile);
        }

#endif
#if !AF_USE_MIX
        int audioVolume = static_cast<int>(mVolume * 100);

        if ((audioVolume >= 0 && audioVolume < 100) || (audioVolume > 100 && audioVolume <= 200)) {
            if (audioVolume == 0) {
                memset(pBuffer, 0, size);
            } else {
                if (mAudioFormat.mBitsPerChannel == 16) {
                    CHANGE(short, 2, 32768)
                } else if (mAudioFormat.mBitsPerChannel == 8) {
                    CHANGE(char, 1, 256)
                }
            }
        }

#endif
    }

    void AFAudioUnitRender::stopInUnit()
    {
        AF_LOGI("stopInUnit:%p", this);

        if (mUnitMutex.try_lock()) {
            if (nil != mAudioUnit && !mRunning && mDeviceWorking) {
                OSErr iErrCode = AudioOutputUnitStop(mAudioUnit);

                if (noErr != iErrCode) {
                    mUnitMutex.unlock();
                    AF_LOGE("stopInUnit AudioOutputUnitStop:%d", iErrCode);
                    return;
                }

                mDeviceWorking = false;
                AF_LOGI("stopInUnit success:%p", this);
            }

            mUnitMutex.unlock();
        }
    }

    int AFAudioUnitRender::device_write(unique_ptr<IAFFrame> &frame)
    {
        if ((QUEUE_CAPACITY - mMaxListSize) >= mAudioDataList.write_available()) {
            mNeedWaitDateFull = false;
            return -EAGAIN;
        }

        if (mPacketDuration != frame->getInfo().duration) {
            mPacketDuration = frame->getInfo().duration;
        }

        auto *avafFrame = dynamic_cast<AVAFFrame *>(frame.release());
        assert(avafFrame != nullptr);
        int listSize = mLastFetchSize / getPCMFrameLen(avafFrame->ToAVFrame()) + 2;

        if (listSize > mMaxListSize) {
            mMaxListSize = listSize > QUEUE_CAPACITY ? QUEUE_CAPACITY : listSize;
        }

        mAudioDataList.push(avafFrame);
        return 0;
    }

    int AFAudioUnitRender::onInterruption(AF_AUDIO_SESSION_STATUS status)
    {
        switch (status) {
            case AFAudioSessionEndInterruption: {
                AF_LOGI("AudioInterruption:%d", (int)mNeedRerun);

                if (mNeedRerun) {
#if TARGET_OS_IPHONE
                    AFAudioSessionWrapper::activeAudio();
#endif
                    pause(false);

                    if (mListener) {
                        mListener->onInterrupt(false);
                    }
                }
            }
            break;

            case AFAudioSessionBeginInterruption: {
                std::unique_lock<std::recursive_mutex> locker(mUnitMutex);

                if (mRunning) {
                    mNeedRerun = true;
                }

                pause(true);
                stopInUnit();
                mDeviceWorking = false;

                // notify only running
                if (mNeedRerun && mListener) {
                    mListener->onInterrupt(true);
                }
            }
            break;

            case AFAudioSessionMediaServicesWereReset: {
                mDeviceWorking = false;
                closeDevice();
                start_device();
            }
            break;

            default:
                break;
        }

        return 0;
    }

    AFAudioUnitRender::AFAudioUnitRender(int dummy)
    {
        addPrototype(this);
        mIsDummy = true;
#if TARGET_OS_IPHONE
        AFAudioSessionWrapper::init();
#endif
    }

    uint64_t AFAudioUnitRender::device_get_que_duration()
    {
        return mPacketDuration * mAudioDataList.read_available();
    };

}
