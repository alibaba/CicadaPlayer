#ifndef MEDIACODEC_JNI_HH
#define MEDIACODEC_JNI_HH

#include "mediaCodec.h"
#include <string>

#define BUFFER_FLAG_CODEC_CONFIG  2
#define BUFFER_FLAG_END_OF_STREAM 4
#define INFO_OUTPUT_BUFFERS_CHANGED -3
#define INFO_OUTPUT_FORMAT_CHANGED  -2
#define INFO_TRY_AGAIN_LATER        -1

#define ALL_CODECS       1
#define REGULAR_CODECS   0


namespace Cicada
{

std::string mediaCodecGetName(const char *psz_mime, const mc_args &args,
                              size_t h264_profile, bool *p_adaptive);

class MediaCodec_JNI : public mediaCodec
{
public:

    int init(const char *mime, int category, jobject surface) override;

    int setOutputSurface(jobject surface) override;

    int flush() override;

    int configure(size_t i_h264_profile, const mc_args &args) override;

    int queue_in(int index, const void *p_buf, size_t size, int64_t pts, bool config) override;

    int dequeue_in(int64_t timeout) override;

    int dequeue_out(int64_t timeout) override;

    int start() override;

    int stop() override;

    int get_out(int index, mc_out *out, bool readBuffer = true) override;

    int release_out(int index, bool render) override;

    void unInit() override;

private:
    const char *psz_mime;
    jobject codec{nullptr};
    int category_codec{-1};
    bool b_started;
    jobject mSurface{nullptr};
    jobject buffer_info{nullptr};
    jobjectArray input_buffers{nullptr}, output_buffers{nullptr};
};
}
#endif // MEDIACODEC_JNI_HH
