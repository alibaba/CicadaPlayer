//
// Created by SuperMan on 2020/10/14.
//

#ifndef SOURCE_OUTPUTBUFFERINFO_H
#define SOURCE_OUTPUTBUFFERINFO_H

#include <jni.h>
#include <cstdint>

namespace Cicada {
    class OutputBufferInfo {

    public:
        static void init(JNIEnv *env);

        static void unInit(JNIEnv *env);

        static void convert(JNIEnv *env, OutputBufferInfo *dstInfo, jobject info);

    public:
        int type;
        int index;
        int64_t pts;
        int flags;
        bool eos;

        int bufferSize;
        int bufferOffset;

        int videoWidth;
        int videoHeight;
        int videoStride;
        int videoSliceHeight;
        int videoPixelFormat;
        int videoCropLeft;
        int videoCropRight;
        int videoCropTop;
        int videoCropBottom;

        int audioChannelCount;
        int audioChannelMask;
        int audioSampleRate;
        int audioFormat;
    };
}


#endif //SOURCE_OUTPUTBUFFERINFO_H
