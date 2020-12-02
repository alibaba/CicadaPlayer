//
// Created by SuperMan on 2020/10/13.
//

#ifndef SOURCE_MEDIACODEC_DECODER_H
#define SOURCE_MEDIACODEC_DECODER_H

#include <jni.h>
#include <string>
#include <map>
#include <base/media/IAFPacket.h>

#define MC_ERROR (-1)
#define MC_INFO_OUTPUT_FORMAT_CHANGED (-2)
#define MC_INFO_OUTPUT_BUFFERS_CHANGED (-3)
#define MC_INFO_TRYAGAIN (-11)

#define CATEGORY_VIDEO (0)
#define CATEGORY_AUDIO (1)

namespace Cicada {
    struct mc_out {
        int type;
        bool b_eos;
        union {
            struct {
                int index;
                int64_t pts;
                const uint8_t *p_ptr;
                size_t size;
            } buf;
            union {
                struct {
                    unsigned int width, height;
                    unsigned int stride;
                    unsigned int slice_height;
                    int pixel_format;
                    int crop_left;
                    int crop_top;
                    int crop_right;
                    int crop_bottom;
                } video;
                struct {
                    int channel_count;
                    int channel_mask;
                    int sample_rate;
                    int format;
                } audio;
            } conf;
        };
    };

    class CodecSpecificData {
    public:
        CodecSpecificData() = default;

        void setScd(const std::string& keyStr ,void* data, int size){
            buffer = data;
            len = size;
            key = keyStr;
        }

        ~CodecSpecificData() {
        }

        std::string key{};
        void *buffer = nullptr;
        int len = 0;
    };

    class MediaCodec_Decoder {
    public:
        static void init(JNIEnv *env);

        static void unInit(JNIEnv *env);

    public:

        MediaCodec_Decoder();

        ~MediaCodec_Decoder();

        void setCodecSpecificData(std::list<CodecSpecificData> csds);

        int setDrmInfo(const std::string &uuid, const void *sessionId, int size);

        void setForceInsecureDecoder(bool force);

        int
        configureVideo(const std::string &mime, int width, int height, int angle, void *surface);

        int configureAudio(const std::string &mime, int sampleRate, int channelCount, int isADTS);

        int start();

        int flush();

        int stop();

        int release();

        int dequeueInputBufferIndex(int64_t timeoutUs);

        int queueInputBuffer(int index, void *buffer, size_t size, int64_t pts, bool isConfig);

        int queueSecureInputBuffer(int index, void *buffer, size_t size,
                                   IAFPacket::EncryptionInfo *pEncryptionInfo, int64_t pts,
                                   bool isConfig);

        int dequeueOutputBufferIndex(int64_t timeoutUs);

        int getOutput(int index, mc_out *out, bool readBuffer);

        int releaseOutputBuffer(int index, bool render);

    private:
        jobject mMediaCodec{nullptr};

        int mCodecCategory{CATEGORY_VIDEO};
    };

}

#endif //SOURCE_MEDIACODEC_DECODER_H
