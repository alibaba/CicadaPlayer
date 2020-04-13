//
// Created by lifujun on 2019/8/29.
//
#define LOG_TAG "AudioTrackRender"
#include "AudioTrackRender.h"

#include <cerrno>
#include <cassert>
#include <utils/ffmpeg_utils.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/JniException.h>
#include <utils/Android/AndroidJniHandle.h>


const int PLAYSTATE_PAUSED = 2;  // matches SL_PLAYSTATE_PAUSED
/** indicates AudioTrack state is playing */
const int PLAYSTATE_PLAYING = 3;  // matches SL_PLAYSTATE_PLAYING


using namespace Cicada;

AudioTrackRender AudioTrackRender::se(0);

AudioTrackRender::AudioTrackRender()
{
}

AudioTrackRender::~AudioTrackRender()
{
    JniEnv  jniEnv;
    JNIEnv *handle = jniEnv.getEnv();

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

int AudioTrackRender::init_device()
{
    adjustOutputInfo();
    int ret = init_jni();

    if (ret < 0) {
        return ret;
    }

    mSimpleSize = mOutputInfo.nb_samples;
    return 0;
}

void AudioTrackRender::adjustOutputInfo()
{
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
    AndroidJniHandle<jobject> audio_track_tmp(handle->NewObject(audio_track_impl,
            constructor_id,
            3,             /*AudioManager.STREAM_MUSIC*/
            mOutputInfo.sample_rate,   /*sampleRateInHz*/
            channelType,   /*CHANNEL_CONFIGURATION_STEREO*/
            2,             /*ENCODING_PCM_16BIT*/
            buffer_size,   /*bufferSizeInBytes*/
            1              /*AudioTrack.MODE_STREAM*/
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

        return 0;
    }

    return -1;
}


void AudioTrackRender::flush_device_inner()
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
    /* work around some device position didn't set to zero, and MUST get after start_device */
    start_device();
    if (mFlushPositionReset == FlushRestPosition::unknow) {
        mFlushPositionReset = (getDevicePlayedSimples() == 0) ? FlushRestPosition::reset
                                                              : FlushRestPosition::notReset;
    }

    if (state == PLAYSTATE_PAUSED) {
        pause_device();
    }
}


void AudioTrackRender::flush_device()
{
    flush_device_inner();
    mBasePlayedPosition = 0;
    mAudioFlushPosition = getPlayedPosition();
}

void AudioTrackRender::device_setVolume(float gain)
{
    if (audio_track && method_setVolume) {
        JniEnv  jniEnv;
        JNIEnv *handle = jniEnv.getEnv();
        handle->CallIntMethod(audio_track, method_setVolume, gain, gain);
    }
}

int64_t AudioTrackRender::device_get_position()
{
    uint64_t playedSimples = getPlayedPosition();
    return static_cast<int64_t>((playedSimples - mAudioFlushPosition ) / (float(mOutputInfo.sample_rate) / 1000000));
}

uint64_t AudioTrackRender::getPlayedPosition() {
    return getDevicePlayedSimples() + mBasePlayedPosition;
}

uint64_t AudioTrackRender::getDevicePlayedSimples()
{
    if(mFlushPositionReset == FlushRestPosition ::notReset) {
        return static_cast<uint64_t>(mSendSimples);
    }

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
    if(mFlushPositionReset != FlushRestPosition::notReset) {
        uint64_t playedSamples = getDevicePlayedSimples();
        if (playedSamples >= 0x7F000000) {
            uint64_t deviceQueDuration = device_get_que_duration();
            if (deviceQueDuration > 0) {
                //will over flow , wait to que buffer be played
                return -EAGAIN;
            } else {
                mBasePlayedPosition += playedSamples;
                flush_device_inner();
            }
        }
    }

    IAFFrame::audioInfo *audioInfo = &(frame->getInfo().audio);
    assert(audioInfo->format == AF_SAMPLE_FMT_S16);
    JniEnv  jniEnv;
    JNIEnv *handle = jniEnv.getEnv();

    if (audioInfo->nb_samples != mSimpleSize) {
        if (jbuffer != nullptr) {
            handle->DeleteGlobalRef(jbuffer);
            jbuffer = nullptr;
        }

        mSimpleSize = audioInfo->nb_samples;
    }

    int len = getPCMDataLen(audioInfo->channels, static_cast<AVSampleFormat>(audioInfo->format), audioInfo->nb_samples);

    if (jbuffer == nullptr) {
        jbuffer = handle->NewGlobalRef(handle->NewByteArray(len));
    }

    if (audio_track && method_write) {
        handle->SetByteArrayRegion(static_cast<jbyteArray>(jbuffer), 0, len, (jbyte *) frame->getData()[0]);
        handle->CallIntMethod(audio_track, method_write, jbuffer, 0, len);
        mSendSimples += mSimpleSize;
    }

    frame = nullptr;
    return 0;
}

uint64_t AudioTrackRender::device_get_que_duration()
{
    if (mSendSimples < getDevicePlayedSimples()) {
        return 0;
    }

    return static_cast<uint64_t>((mSendSimples - getDevicePlayedSimples()) /
                                 (float(mOutputInfo.sample_rate) / 1000000));
}
