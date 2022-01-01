//
// Created by SuperMan on 2020/10/13.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/JniUtils.h>
#include <utils/Android/NewByteArray.h>
#include <utils/frame_work_log.h>
#include <utils/Android/NewHashMap.h>
#include "MediaCodec_Decoder.h"
#include "OutputBufferInfo.h"
#include "JEncryptionInfo.h"

using namespace Cicada;

static jclass jMediaCodecClass = nullptr;
static jmethodID jMediaCodec_init = nullptr;
static jmethodID jMediaCodec_setCodecSpecificData = nullptr;
static jmethodID jMediaCodec_setDrmInfo = nullptr;
static jmethodID jMediaCodec_setForceInsecureDecoder = nullptr;
static jmethodID jMediaCodec_configureVideo = nullptr;
static jmethodID jMediaCodec_configureAudio = nullptr;
static jmethodID jMediaCodec_start = nullptr;
static jmethodID jMediaCodec_flush = nullptr;
static jmethodID jMediaCodec_stop = nullptr;
static jmethodID jMediaCodec_release = nullptr;
static jmethodID jMediaCodec_releaseOutputBuffer = nullptr;
static jmethodID jMediaCodec_dequeueInputBufferIndex = nullptr;
static jmethodID jMediaCodec_queueInputBuffer = nullptr;
static jmethodID jMediaCodec_queueSecureInputBuffer = nullptr;
static jmethodID jMediaCodec_dequeueOutputBufferIndex = nullptr;
static jmethodID jMediaCodec_getOutputBufferInfo = nullptr;
static jmethodID jMediaCodec_getOutBuffer = nullptr;

void MediaCodec_Decoder::init(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }

    if (jMediaCodecClass == nullptr) {
        FindClass jClass(env, "com/cicada/player/utils/media/MediaCodecDecoder");
        jMediaCodecClass = static_cast<jclass>(env->NewGlobalRef(jClass.getClass()));
        jMediaCodec_init = env->GetMethodID(jMediaCodecClass, "<init>", "()V");
        jMediaCodec_setCodecSpecificData = env->GetMethodID(jMediaCodecClass,
                                                            "setCodecSpecificData", "(Ljava/lang/Object;)V");
        jMediaCodec_setDrmInfo = env->GetMethodID(jMediaCodecClass, "setDrmInfo",
                                                  "(Ljava/lang/String;[B)Z");
        jMediaCodec_setForceInsecureDecoder = env->GetMethodID(jMediaCodecClass, "setForceInsecureDecoder",
                                                  "(Z)V");
        jMediaCodec_configureVideo = env->GetMethodID(jMediaCodecClass, "configureVideo",
                                                      "(Ljava/lang/String;IIILjava/lang/Object;)I");
        jMediaCodec_configureAudio = env->GetMethodID(jMediaCodecClass, "configureAudio",
                                                      "(Ljava/lang/String;III)I");
        jMediaCodec_start = env->GetMethodID(jMediaCodecClass, "start", "()I");
        jMediaCodec_flush = env->GetMethodID(jMediaCodecClass, "flush", "()I");
        jMediaCodec_stop = env->GetMethodID(jMediaCodecClass, "stop", "()I");
        jMediaCodec_release = env->GetMethodID(jMediaCodecClass, "release", "()I");
        jMediaCodec_releaseOutputBuffer = env->GetMethodID(jMediaCodecClass, "releaseOutputBuffer",
                                                           "(IZ)I");
        jMediaCodec_dequeueInputBufferIndex = env->GetMethodID(jMediaCodecClass,
                                                               "dequeueInputBufferIndex", "(J)I");
        jMediaCodec_queueInputBuffer = env->GetMethodID(jMediaCodecClass, "queueInputBuffer",
                                                        "(I[BJZ)I");
        jMediaCodec_queueSecureInputBuffer = env->GetMethodID(jMediaCodecClass,
                                                              "queueSecureInputBuffer",
                                                              "(I[BLjava/lang/Object;JZ)I");
        jMediaCodec_dequeueOutputBufferIndex = env->GetMethodID(jMediaCodecClass,
                                                                "dequeueOutputBufferIndex", "(J)I");
        jMediaCodec_getOutputBufferInfo = env->GetMethodID(jMediaCodecClass, "getOutputBufferInfo",
                                                           "(I)Ljava/lang/Object;");
        jMediaCodec_getOutBuffer = env->GetMethodID(jMediaCodecClass, "getOutBuffer",
                                                    "(I)Ljava/lang/Object;");
    }


}

void MediaCodec_Decoder::unInit(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }
    if (jMediaCodecClass != nullptr) {
        env->DeleteGlobalRef(jMediaCodecClass);
        jMediaCodecClass = nullptr;
    }
}


MediaCodec_Decoder::MediaCodec_Decoder() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    jobject pJobject = env->NewObject(jMediaCodecClass, jMediaCodec_init);
    mMediaCodec = env->NewGlobalRef(pJobject);
    env->DeleteLocalRef(pJobject);
}

MediaCodec_Decoder::~MediaCodec_Decoder() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    if (mMediaCodec != nullptr) {
        env->DeleteGlobalRef(mMediaCodec);
    }
}

void MediaCodec_Decoder::setCodecSpecificData(std::list<CodecSpecificData > csds) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return;
    }

    if(csds.empty()){
        return;
    }

    NewHashMap csdMap(env);

    for (CodecSpecificData & data: csds) {
        std::string key = data.key;

        NewStringUTF keyStr(env, key.c_str());
        NewByteArray csdData(env , data.buffer, data.len);
        csdMap.put(keyStr.getString(),csdData.getArray());
    }

    env->CallVoidMethod(mMediaCodec, jMediaCodec_setCodecSpecificData, csdMap.getMap());

}

int MediaCodec_Decoder::setDrmInfo(const std::string &uuid, const void *sessionId, int size) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    NewStringUTF jKeyUrl(env, uuid.c_str());
    NewByteArray jSessionId(env, sessionId, size);
    bool ret = env->CallBooleanMethod(mMediaCodec, jMediaCodec_setDrmInfo, jKeyUrl.getString(),
                                      jSessionId.getArray());

    return ret? 0 : MC_ERROR;
}

void MediaCodec_Decoder::setForceInsecureDecoder(bool force)
{
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return ;
    }

    env->CallVoidMethod(mMediaCodec, jMediaCodec_setForceInsecureDecoder,  (jboolean)force);

}

int MediaCodec_Decoder::configureVideo(const std::string &mime, int width, int height, int angle,
                                       void *surface) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    mCodecCategory = CATEGORY_VIDEO;
    NewStringUTF jMime(env, mime.c_str());
    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_configureVideo, jMime.getString(),
                                 (jint) width, (jint) height, (jint) angle, (jobject) surface);

    return ret;
}

int MediaCodec_Decoder::configureAudio(const std::string &mime, int sampleRate, int channelCount,
                                       int isADTS) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    mCodecCategory = CATEGORY_AUDIO;
    NewStringUTF jMime(env, mime.c_str());
    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_configureAudio, jMime.getString(),
                                 (jint) sampleRate, (jint) channelCount, (jint) isADTS);

    return ret;
}

int MediaCodec_Decoder::start() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_start);

    return ret;
}

int MediaCodec_Decoder::flush() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_flush);

    return ret;
}

int MediaCodec_Decoder::stop() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_stop);

    return ret;
}

int MediaCodec_Decoder::release() {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_release);

    return ret;
}

int MediaCodec_Decoder::dequeueInputBufferIndex(int64_t timeoutUs) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_dequeueInputBufferIndex,
                                 (jlong) timeoutUs);
    return ret;
}

int MediaCodec_Decoder::dequeueOutputBufferIndex(int64_t timeoutUs) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_dequeueOutputBufferIndex,
                                 (jlong) timeoutUs);

    return ret;
}

int MediaCodec_Decoder::queueInputBuffer(int index, void *buffer, size_t size, int64_t pts,
                                         bool isConfig) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    jbyteArray jBuffer = nullptr;
    if (buffer != nullptr) {
        jBuffer = env->NewByteArray(size);
        env->SetByteArrayRegion(jBuffer, 0, size, (jbyte *) (buffer));
    }


    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_queueInputBuffer, (jint) index, jBuffer,
                                 (jlong) pts, (jboolean) isConfig);
    if (jBuffer != nullptr) {
        env->DeleteLocalRef(jBuffer);
    }
    return ret;
}

int MediaCodec_Decoder::queueSecureInputBuffer(int index, void *buffer, size_t size,
                                               IAFPacket::EncryptionInfo *pEncryptionInfo,
                                               int64_t pts, bool isConfig) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    jbyteArray jBuffer = nullptr;
    if (buffer != nullptr) {
        jBuffer = env->NewByteArray(size);
        env->SetByteArrayRegion(jBuffer, 0, size, (jbyte *) (buffer));
    }

    jobject encryptionInfo = nullptr;
    if (pEncryptionInfo != nullptr) {
        encryptionInfo = JEncryptionInfo::convert(env, pEncryptionInfo);
    }
    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_queueSecureInputBuffer, (jint) index,
                                 jBuffer,
                                 encryptionInfo, (jlong) pts, (jboolean) isConfig);
    if (jBuffer != nullptr) {
        env->DeleteLocalRef(jBuffer);
    }

    if(encryptionInfo != nullptr){
        env->DeleteLocalRef(encryptionInfo);
    }

    return ret;
}

int MediaCodec_Decoder::getOutput(int index, mc_out *out, bool readBuffer) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    OutputBufferInfo outputBufferInfo{};

    jobject outputInfo = env->CallObjectMethod(mMediaCodec, jMediaCodec_getOutputBufferInfo,
                                               (jint) index);
    if (outputInfo != nullptr) {
        OutputBufferInfo::convert(env, &outputBufferInfo, outputInfo);
        env->DeleteLocalRef(outputInfo);
    }

    if (index >= 0) {
        out->type = outputBufferInfo.type;
        out->b_eos = outputBufferInfo.eos;
        out->buf.index = outputBufferInfo.index;
        out->buf.pts = outputBufferInfo.pts;

        if (readBuffer) {
            jobject bufferInfo = env->CallObjectMethod(mMediaCodec, jMediaCodec_getOutBuffer,
                                                       (jint) index);
            if (bufferInfo != nullptr) {
                uint8_t *ptr = (uint8_t *) env->GetDirectBufferAddress(bufferInfo);
                int offset = outputBufferInfo.bufferOffset;
                out->buf.p_ptr = ptr + offset;
                out->buf.size = outputBufferInfo.bufferSize;

                env->DeleteLocalRef(bufferInfo);
            }
        } else {
            out->buf.p_ptr = nullptr;
            out->buf.size = 0;
        }

    } else if (index == MC_INFO_OUTPUT_FORMAT_CHANGED) {
        out->type = outputBufferInfo.type;
        out->b_eos = outputBufferInfo.eos;

        if (mCodecCategory == CATEGORY_VIDEO) {
            out->conf.video.width = outputBufferInfo.videoWidth;
            out->conf.video.height = outputBufferInfo.videoHeight;
            out->conf.video.stride = outputBufferInfo.videoStride;
            out->conf.video.slice_height = outputBufferInfo.videoSliceHeight;
            out->conf.video.pixel_format = outputBufferInfo.videoPixelFormat;
            out->conf.video.crop_left = outputBufferInfo.videoCropLeft;
            out->conf.video.crop_top = outputBufferInfo.videoCropTop;
            out->conf.video.crop_right = outputBufferInfo.videoCropRight;
            out->conf.video.crop_bottom = outputBufferInfo.videoCropBottom;
        } else {
            out->conf.audio.channel_count = outputBufferInfo.audioChannelCount;
            out->conf.audio.channel_mask = outputBufferInfo.audioChannelMask;
            out->conf.audio.sample_rate = outputBufferInfo.audioSampleRate;
            out->conf.audio.format = outputBufferInfo.audioFormat;
        }
    }


    return 0;
}

int MediaCodec_Decoder::releaseOutputBuffer(int index, bool render) {
    JniEnv jniEnv{};

    JNIEnv *env = jniEnv.getEnv();
    if (env == nullptr) {
        return MC_ERROR;
    }

    if (mMediaCodec == nullptr) {
        return MC_ERROR;
    }

    int ret = env->CallIntMethod(mMediaCodec, jMediaCodec_releaseOutputBuffer, (jint) index,
                                 (jboolean) render);
    return ret;
}



