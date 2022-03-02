//
// Created by lifujun on 2019/8/29.
//
#define LOG_TAG "AudioTrackRender"
#include "AudioTrackRender.h"
#include <utils/frame_work_log.h>

#include <cassert>
#include <cerrno>
#include <utils/Android/AndroidJniHandle.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/JniException.h>
#include <utils/ffmpeg_utils.h>
#include <utils/globalSettings.h>
#include <utils/timer.h>


const int PLAYSTATE_PAUSED = 2;  // matches SL_PLAYSTATE_PAUSED
/** indicates AudioTrack state is playing */
const int PLAYSTATE_PLAYING = 3;  // matches SL_PLAYSTATE_PLAYING

#define MAX_FRAME_QUEUE_SIZE 16
#define MIN_FRAME_QUEUE_SIZE 2


using namespace Cicada;

AudioTrackRender AudioTrackRender::se(0);

AudioTrackRender::AudioTrackRender() : mFrameQueue(MAX_FRAME_QUEUE_SIZE)
{
}

AudioTrackRender::~AudioTrackRender()
{
    JniEnv  jniEnv;
    JNIEnv *handle = jniEnv.getEnv();

    mRunning = false;

    delete mWriteThread;

    while (!mFrameQueue.empty()) {
        delete mFrameQueue.front();
        mFrameQueue.pop();
    }

    if (handle) {
        if (audio_track && method_stop) {
            handle->CallVoidMethod(audio_track, method_stop);

            if (JniException::clearException(handle)) {
                AF_LOGE("AudioTrack stop exception. maybe IllegalStateException.");
            }
        }

        if (audio_track && method_release) {
            handle->CallVoidMethod(audio_track, method_release);
            handle->DeleteGlobalRef(audio_track);
        }

        if (jbuffer != nullptr) {
            handle->DeleteGlobalRef(jbuffer);
        }
    }

    if (mPcmBuffer) {
        free(mPcmBuffer);
        mPcmBuffer = nullptr;
    }
}

bool AudioTrackRender::device_require_format(const IAFFrame::audioInfo &info)
{
    if (info.format != AF_SAMPLE_FMT_S16 || info.sample_rate > 48000 || info.channels > 2) {
        return false;
    }
    mRequireFormat = std::unique_ptr<IAFFrame::audioInfo>(new IAFFrame::audioInfo());
    (*mRequireFormat) = info;
    return true;
}

int AudioTrackRender::init_device()
{
    adjustOutputInfo();
    int ret = init_jni();

    if (ret < 0) {
        return ret;
    }

    mWriteThread = NEW_AF_THREAD(write_loop);
    if (mRunning) {
        mWriteThread->start();
    }
    return 0;
}

void AudioTrackRender::adjustOutputInfo()
{
    if (mRequireFormat) {
        mOutputInfo = *mRequireFormat;
        return;
    }
    if (mInputInfo.format != AF_SAMPLE_FMT_S16) {
        mOutputInfo.format = AF_SAMPLE_FMT_S16;
        needFilter = true;
    }

    if (mInputInfo.sample_rate > 48000) {
        mOutputInfo.sample_rate = 48000;
        needFilter = true;
    }

    if (mInputInfo.channels > 2) {
        mOutputInfo.channels = 2;
        needFilter = true;
    }
}


int AudioTrackRender::init_jni()
{
    const int CHANNEL_CONFIGURATION_MONO = 4;
    const int CHANNEL_CONFIGURATION_STEREO = 12;
    int channelType =
        mOutputInfo.channels == 2 ? CHANNEL_CONFIGURATION_STEREO : CHANNEL_CONFIGURATION_MONO;
    JniEnv  jniEnv;
    JNIEnv *handle = jniEnv.getEnv();

    if (!handle) {
        AF_LOGE("init jni error \n");
        return -ENOTSUP;
    }

    AndroidJniHandle<jclass> audio_track_cls(handle->FindClass("android/media/AudioTrack"));
    jmethodID min_buff_size_id = handle->GetStaticMethodID(
                                     audio_track_cls,
                                     "getMinBufferSize",
                                     "(III)I");
    jclass audio_track_impl;
    jmethodID constructor_id;
    audio_track_impl = audio_track_cls;
    AF_LOGI("choose internal audio track.");
    int buffer_size = handle->CallStaticIntMethod(audio_track_impl, min_buff_size_id,
                      mOutputInfo.sample_rate,
                      channelType, /*CHANNEL_CONFIGURATION_STEREO*/
                      2);          /*ENCODING_PCM_16BIT*/

    if (buffer_size < 0) {
        AF_LOGE("Init failed device not support. sampleRate %d", mOutputInfo.sample_rate);
        return -5;
    }

    constructor_id = handle->GetMethodID(audio_track_impl, "<init>", "(IIIIII)V");
    const string &streamType = Cicada::globalSettings::getSetting().getProperty("audio.streamType");
    int audioStreamType = streamType.empty() ? 3 : atoi(streamType.c_str());
    AndroidJniHandle<jobject> audio_track_tmp(handle->NewObject(audio_track_impl, constructor_id,
                                                                audioStreamType,         /*AudioManager.stream type*/
                                                                mOutputInfo.sample_rate, /*sampleRateInHz*/
                                                                channelType,             /*CHANNEL_CONFIGURATION_STEREO*/
                                                                2,                       /*ENCODING_PCM_16BIT*/
                                                                buffer_size,             /*bufferSizeInBytes*/
                                                                1                        /*AudioTrack.MODE_STREAM*/
                                                                ));

    if (JniException::clearException(handle)) {
        AF_LOGE("audioTrack constructor exception. sample_rate %d, channel %d, bufferSize %d",
                mOutputInfo.sample_rate, channelType, buffer_size);
        return -6;
    }

    audio_track = handle->NewGlobalRef(audio_track_tmp);
    // get method
    method_setVolume = handle->GetMethodID(audio_track_impl, "setStereoVolume", "(FF)I");
    method_play = handle->GetMethodID(audio_track_impl, "play", "()V");
    method_pause = handle->GetMethodID(audio_track_impl, "pause", "()V");
    method_flush = handle->GetMethodID(audio_track_impl, "flush", "()V");
    method_stop = handle->GetMethodID(audio_track_impl, "stop", "()V");
    method_getPlayState = handle->GetMethodID(audio_track_impl, "getPlayState", "()I");
    method_getPosition = handle->GetMethodID(audio_track_impl, "getPlaybackHeadPosition", "()I");
    method_write = handle->GetMethodID(audio_track_impl, "write", "([BII)I");
    method_release = handle->GetMethodID(audio_track_impl, "release", "()V");
    return 0;
}

int AudioTrackRender::pause_device()
{
    if (mWriteThread && std::this_thread::get_id() == mWriteThread->getId()) {
        //same thread
    } else {
        mRunning = false;
        if (mWriteThread) {
            mWriteThread->pause();
        }
    }

    if (audio_track && method_pause) {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();
        handle->CallVoidMethod(audio_track, method_pause);

        if (JniException::clearException(handle)) {
            AF_LOGE("AudioTrack pause exception. maybe IllegalStateException.");
            return -1;
        }

        return 0;
    }

    return -1;
}


int AudioTrackRender::start_device()
{
    if (audio_track && method_play) {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();
        handle->CallVoidMethod(audio_track, method_play);

        if (JniException::clearException(handle)) {
            AF_LOGE("AudioTrack start exception. maybe IllegalStateException.");
            return -1;
        }
    }

    if (mWriteThread && std::this_thread::get_id() == mWriteThread->getId()) {
        //same thread
    } else {
        mRunning = true;
        if (mWriteThread) {
            mWriteThread->start();
        }
    }

    return 0;
}


void AudioTrackRender::flush_device_inner(bool clearFrameQueue)
{
    JniEnv  jniEnv;
    JNIEnv *handle = jniEnv.getEnv();
    int state = 0;

    if (audio_track && method_getPlayState) {
        state = handle->CallIntMethod(audio_track, method_getPlayState);
    }

    if (state == PLAYSTATE_PLAYING) {
        pause_device();
    }

    if (audio_track && method_flush) {
        handle->CallVoidMethod(audio_track, method_flush);

        if (JniException::clearException(handle)) {
            AF_LOGE("AudioTrack flush exception. maybe IllegalStateException.");
        }
    }

    mSendSimples = 0;

    if (clearFrameQueue) {
        while (!mFrameQueue.empty()) {
            delete mFrameQueue.front();
            mFrameQueue.pop();
        }
    }

    mMaxQueSize = 2;

    /* work around some device position didn't set to zero, and MUST get after start_device */
    start_device();

    mAudioFlushPosition = getDevicePlayedSimples();

    if (state == PLAYSTATE_PAUSED) {
        pause_device();
    }
}


void AudioTrackRender::flush_device()
{
    flush_device_inner(true);
}

void AudioTrackRender::device_setVolume(float gain)
{
    mVolume = gain;
    if (mMute) {
        return;
    }
    if (audio_track && method_setVolume) {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();
        handle->CallIntMethod(audio_track, method_setVolume, gain, gain);
    }
}
void AudioTrackRender::device_mute(bool bMute)
{
    mMute = bMute;
    if (bMute) {
        if (audio_track && method_setVolume) {
            JniEnv jniEnv;
            JNIEnv *handle = jniEnv.getEnv();
            handle->CallIntMethod(audio_track, method_setVolume, 0.0f, 0.0f);
        }
    } else {
        device_setVolume(mVolume);
    }
}

int64_t AudioTrackRender::device_get_position()
{
    uint64_t playedSimples = getDevicePlayedSimples() - mAudioFlushPosition;
    int64_t position = static_cast<int64_t>((playedSimples) / (float(mOutputInfo.sample_rate) / 1000000));
    return position;
}

uint64_t AudioTrackRender::getDevicePlayedSimples()
{

    uint64_t simples = 0;
    JniEnv  jniEnv;
    JNIEnv *handle = jniEnv.getEnv();

    if (handle) {
        int state = 0;

        if (audio_track && method_getPlayState) {
            state = handle->CallIntMethod(audio_track, method_getPlayState);
        }

        if (state == PLAYSTATE_PLAYING || state == PLAYSTATE_PAUSED) {
            if (audio_track && method_getPosition) {
                simples = static_cast<uint64_t>(handle->CallIntMethod(audio_track,
                                                method_getPosition));
            } else {
                return static_cast<uint64_t>(mSendSimples);
            }
        }
    }

    return simples;
}


int AudioTrackRender::device_write(unique_ptr<IAFFrame> &frame)
{
    //    AF_LOGD("xxxxxx mFrameQueue.size() is %d\n", mFrameQueue.size());
    if (mFrameQueue.size() >= mMaxQueSize) {
        //    mMaxQueSize = std::max(MIN_FRAME_QUEUE_SIZE,mMaxQueSize -1);
        return -EAGAIN;
    }
    if (frame == nullptr) {
        return 0;
    }
    mFrameQueue.push(frame.release());
    return 0;
}
int AudioTrackRender::write_loop()
{
    if (mFrameQueue.empty()) {
        af_msleep(5);
        mMaxQueSize = std::min(mMaxQueSize + 1, MAX_FRAME_QUEUE_SIZE);
        return 0;
    }
    while (!mFrameQueue.empty() && mRunning) {
        int ret = device_write_internal(mFrameQueue.front());
        if (ret == -EAGAIN) {
            af_msleep(5);
            //            break;
        } else {
            if (mFrameQueue.size() >= mMaxQueSize) {
                // TODO: How to decrease mMaxQueSize size, if use nonblock write, we can do this
                //     mMaxQueSize = std::max(MIN_FRAME_QUEUE_SIZE,mMaxQueSize -1);
            }
            if (mListener) {
                mListener->onFrameInfoUpdate(mFrameQueue.front()->getInfo(), true);
            }
            delete mFrameQueue.front();
            mFrameQueue.pop();
        }
    }
    return 0;
}

#define OVERFLOW_SAMPLES (0x7F000000)

int AudioTrackRender::device_write_internal(IAFFrame *frame)
{
    uint64_t playedSamples = getDevicePlayedSimples();
    if (playedSamples >= OVERFLOW_SAMPLES) {
        uint64_t deviceQueDuration = getDeviceQuequedDuration();
        AF_LOGW("device_write_internal() will over flow  deviceQueDuration= %lld", deviceQueDuration);
        if (deviceQueDuration > 0) {
            //will over flow , wait to que buffer be played
            return -EAGAIN;
        } else {
            flush_device_inner(false);
            //FIXME : Some devices position is not be reset to 0 after flush, need create new AudioTrack.
        }
    }

    IAFFrame::audioInfo *audioInfo = &(frame->getInfo().audio);
    assert(audioInfo->format == AF_SAMPLE_FMT_S16);
    JniEnv  jniEnv;
    JNIEnv *handle = jniEnv.getEnv();

    int len = getPCMDataLen(audioInfo->channels, static_cast<AVSampleFormat>(audioInfo->format), audioInfo->nb_samples);

    if (len > jBufferLen) {
        if (jbuffer != nullptr) {
            handle->DeleteGlobalRef(jbuffer);
            jbuffer = nullptr;
        }
        jBufferLen = len;
    }

    if (jbuffer == nullptr) {
        jbyteArray obj = handle->NewByteArray(jBufferLen);
        jbuffer = handle->NewGlobalRef(obj);
        handle->DeleteLocalRef(obj);
    }

    bool rendered = false;
    if (mRenderingCb != nullptr) {
        rendered = mRenderingCb(mRenderingCbUserData, frame);
    }

    if (audio_track && method_write) {
        handle->SetByteArrayRegion(static_cast<jbyteArray>(jbuffer), 0, len, (jbyte *) frame->getData()[0]);
        handle->CallIntMethod(audio_track, method_write, jbuffer, 0, len);
        mSendSimples += audioInfo->nb_samples;
    }
    return 0;
}

uint64_t AudioTrackRender::device_get_que_duration()
{
    uint64_t duration = 0;
    if (!mFrameQueue.empty()) {
        duration += static_cast<uint64_t>(mFrameQueue.front()->getInfo().duration * mFrameQueue.size());
    }

    uint64_t playedSimples = getDevicePlayedSimples() - mAudioFlushPosition;
    if (mSendSimples < playedSimples) {
        return duration;
    }

    return static_cast<uint64_t>(duration + (mSendSimples - playedSimples) / (float(mOutputInfo.sample_rate) / 1000000));
}

uint64_t AudioTrackRender::getDeviceQuequedDuration()
{
    uint64_t playedSimples = getDevicePlayedSimples() - mAudioFlushPosition;
    if (mSendSimples < playedSimples) {
        return 0;
    }

    return static_cast<uint64_t>((mSendSimples - playedSimples) / (float(mOutputInfo.sample_rate) / 1000000));
}

void AudioTrackRender::device_preClose()
{

    mRunning = false;

    if (mWriteThread) {
        if (std::this_thread::get_id() == mWriteThread->getId()) {
            //same thread
        } else {
            mWriteThread->prePause();
        }
    }
}
