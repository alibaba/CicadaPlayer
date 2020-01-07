#ifndef MEDIACODEC_HH
#define MEDIACODEC_HH

#include "utils/Android/AndroidJniHandle.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

#define GOT_EOS   111


#define PROFILE_H264_BASELINE             66
#define PROFILE_H264_MAIN                 77
#define PROFILE_H264_EXTENDED             88
#define PROFILE_H264_HIGH                 100
#define PROFILE_H264_HIGH_10              110
#define PROFILE_H264_HIGH_422             122
#define PROFILE_H264_HIGH_444             144
#define PROFILE_H264_HIGH_444_PREDICTIVE  244

#define PROFILE_H264_CAVLC_INTRA          44
#define PROFILE_H264_SVC_BASELINE         83
#define PROFILE_H264_SVC_HIGH             86
#define PROFILE_H264_MVC_STEREO_HIGH      128
#define PROFILE_H264_MVC_MULTIVIEW_HIGH   118

#define PROFILE_H264_MFC_HIGH                          134
#define PROFILE_H264_MVC_MULTIVIEW_DEPTH_HIGH          138
#define PROFILE_H264_MVC_ENHANCED_MULTIVIEW_DEPTH_HIGH 139

#define COLOR_FORMAT_YUV420Planar_VU 19
#define COLOR_FORMAT_YUV420SemiPlanar 21
#define COLOR_FORMAT_YUVYCbYCr        25
#define COLOR_FORMAT_YUV420PackedSemiPlanar 39
#define COLOR_FORMAT_OMX_QCOM_YUV420PackedSemiPlanar64x32Tile2m8ka 2141391875
#define COLOR_FORMAT_OMX_QCOM_YUV420PackedSemiPlanar32m 2141391876

#define MC_ERROR -1
#define MC_INFO_TRYAGAIN -11
#define MC_INFO_OUTPUT_FORMAT_CHANGED (-12)
#define MC_INFO_OUTPUT_BUFFERS_CHANGED (-13)

/* in sync with OMXCODEC QUIRKS */
#define MC_NO_QUIRKS 0
#define MC_QUIRKS_NEED_CSD 0x1
#define MC_VIDEO_QUIRKS_IGNORE_PADDING 0x2
#define MC_VIDEO_QUIRKS_SUPPORT_INTERLACED 0x4
#define MC_OUT_TYPE_BUF 0
#define MC_OUT_TYPE_CONF 1

#define CATEGORY_VIDEO 0
#define CATEGORY_AUDIO 1
/* MediaCodec only QUIRKS */
#define MC_VIDEO_QUIRKS_ADAPTIVE 0x1000

#ifndef __MIN
#   define __MIN(a, b)   ( ((a) < (b)) ? (a) : (b) )
#endif

namespace Cicada{

    union mc_args {
        struct {
            int width;
            int height;
            int angle;
        } video;
        struct {
            int sample_rate;
            int channel_count;
        } audio;
    };

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
                } audio;
            } conf;
        };
    };

/**
* AVC profile types, each profile indicates support for various
* performance bounds and different annexes.
*/
    typedef enum OMX_VIDEO_AVCPROFILETYPE {
        OMX_VIDEO_AVCProfileBaseline = 0x01,   /**< Baseline profile */
        OMX_VIDEO_AVCProfileMain = 0x02,   /**< Main profile */
        OMX_VIDEO_AVCProfileExtended = 0x04,   /**< Extended profile */
        OMX_VIDEO_AVCProfileHigh = 0x08,   /**< High profile */
        OMX_VIDEO_AVCProfileHigh10 = 0x10,   /**< High 10 profile */
        OMX_VIDEO_AVCProfileHigh422 = 0x20,   /**< High 4:2:2 profile */
        OMX_VIDEO_AVCProfileHigh444 = 0x40,   /**< High 4:4:4 profile */
        OMX_VIDEO_AVCProfileKhronosExtensions = 0x6F000000, /**< Reserved region for introducing Khronos Standard Extensions */
        OMX_VIDEO_AVCProfileVendorStartUnused = 0x7F000000, /**< Reserved region for introducing Vendor Extensions */
        OMX_VIDEO_AVCProfileMax = 0x7FFFFFFF
    } OMX_VIDEO_AVCPROFILETYPE;

    size_t convert_omx_to_profile_idc(OMX_VIDEO_AVCPROFILETYPE profile_type);

    bool OMXCodec_IsBlacklisted(const char *p_name, unsigned int i_name_len);

    OMX_VIDEO_AVCPROFILETYPE selectprofiletype(int profile);

    class mediaCodec {
    public:
        virtual ~mediaCodec() = default;

        virtual int init(const char *mime, int category, jobject surface) = 0;

        virtual void unInit() = 0;

        virtual int setOutputSurface(jobject surface) = 0;

        virtual int configure(size_t i_h264_profile, const mc_args &args) = 0;

        virtual int start() = 0;

        virtual int stop() = 0;

        virtual int flush() = 0;

        virtual int dequeue_in(int64_t timeout) = 0;

        virtual int dequeue_out(int64_t timeout) = 0;

        virtual int
        queue_in(int index, const void *p_buf, size_t size, int64_t pts, bool config) = 0;

        virtual int get_out(int index, mc_out *out, bool readBuffer = true) = 0;

        virtual int release_out(int index, bool render) = 0;
    };
}

#endif // MEDIACODEC_HH
