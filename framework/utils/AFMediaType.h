/*
 * Copyright (c) 2012 pingkai010@gmail.com
 *
 *
 */
#ifndef CICADA_TYPE_H
#define CICADA_TYPE_H

#include <stdint.h>

typedef enum {
    DEMUX_MODE_NORMOL,
    DEMUX_MODE_I_FRAME,
} Demux_mode_e;

typedef enum {
    STREAM_TYPE_UNKNOWN = -1,
    STREAM_TYPE_VIDEO,
    STREAM_TYPE_AUDIO,
    STREAM_TYPE_SUB,
    STREAM_TYPE_MIXED,
    STREAM_TYPE_NUM,
} Stream_type;

enum AFCodecID {
    AF_CODEC_ID_NONE,

    AF_CODEC_ID_H264,
    AF_CODEC_ID_MPEG4,
    //    AF_CODEC_ID_RV30,
    //    AF_CODEC_ID_RV40,
    //    AF_CODEC_ID_MPEG2VIDEO,
    //    AF_CODEC_ID_VC1,
    //    AF_CODEC_ID_WMV3,
    //    AF_CODEC_ID_WMV1,
    //    AF_CODEC_ID_WMV2,
    //    AF_CODEC_ID_MSMPEG4V2,
    //    AF_CODEC_ID_DIV311,
    //    AF_CODEC_ID_FLV1,
    //    AF_CODEC_ID_SVQ3,
    //    AF_CODEC_ID_MPEG1VIDEO,
    //    AF_CODEC_ID_VP6,
    AF_CODEC_ID_VP8,
    AF_CODEC_ID_VP9,
    //    AF_CODEC_ID_MJPEG,
    //    AF_CODEC_ID_H263,
    AF_CODEC_ID_AV1,
    AF_CODEC_ID_HEVC,

    AF_CODEC_ID_AAC,
    AF_CODEC_ID_AC3,
    AF_CODEC_ID_EAC3,
    //    AF_CODEC_ID_DTS,
    //    AF_CODEC_ID_DTSE,
    AF_CODEC_ID_MP3,
    //    AF_CODEC_ID_APE,
    //    AF_CODEC_ID_COOK,
    //    AF_CODEC_ID_SIPR,
    //    AF_CODEC_ID_QDM2,
    AF_CODEC_ID_MP2,
    AF_CODEC_ID_MP1,
    AF_CODEC_ID_OPUS,
    //    AF_CODEC_ID_AMR_NB,
    //    AF_CODEC_ID_WMAV2,
    //    AF_CODEC_ID_WMAPRO,
    AF_CODEC_ID_PCM_S16LE,
    AF_CODEC_ID_PCM_S16BE,
    //    AF_CODEC_ID_PCM_BLURAY,
    //    AF_CODEC_ID_ADPCM,
    //    AF_CODEC_ID_PCM_S24LE,
    AF_CODEC_ID_PCM_U8,
    //    AF_CODEC_ID_PCM_MULAW,
    //    AF_CODEC_ID_ATRAC3,
    //    AF_CODEC_ID_VORBIS,
    //    AF_CODEC_ID_ALAC,
    //    AF_CODEC_ID_FLAC,

    AF_CODEC_ID_WEBVTT,
    //    AF_CODEC_ID_TEXT,
    //    AF_CODEC_ID_SSA,
    //    AF_CODEC_ID_SRT,
};


enum AFSampleFormat {
    AF_SAMPLE_FMT_NONE = -1,
    AF_SAMPLE_FMT_U8,          ///< unsigned 8 bits
    AF_SAMPLE_FMT_S16,         ///< signed 16 bits
    AF_SAMPLE_FMT_S32,         ///< signed 32 bits
    AF_SAMPLE_FMT_FLT,         ///< float
    AF_SAMPLE_FMT_DBL,         ///< double

    AF_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
    AF_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
    AF_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
    AF_SAMPLE_FMT_FLTP,        ///< float, planar
    AF_SAMPLE_FMT_DBLP,        ///< double, planar

    AF_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};
enum AFPixelFormat {
    AF_PIX_FMT_NONE = -1,
    AF_PIX_FMT_YUV420P,  ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    AF_PIX_FMT_YUYV422,  ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    AF_PIX_FMT_RGB24,    ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    AF_PIX_FMT_BGR24,    ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    AF_PIX_FMT_YUV422P,  ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    AF_PIX_FMT_YUV444P,  ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    AF_PIX_FMT_YUV410P,  ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    AF_PIX_FMT_YUV411P,  ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    AF_PIX_FMT_GRAY8,    ///<        Y        ,  8bpp
    AF_PIX_FMT_MONOWHITE,///<        Y        ,  1bpp, 0 is white, 1 is black, in each byte pixels are ordered from the msb to the lsb
    AF_PIX_FMT_MONOBLACK,///<        Y        ,  1bpp, 0 is black, 1 is white, in each byte pixels are ordered from the msb to the lsb
    AF_PIX_FMT_PAL8,     ///< 8 bits with AV_PIX_FMT_RGB32 palette
    AF_PIX_FMT_YUVJ420P, ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV420P and setting color_range
    AF_PIX_FMT_YUVJ422P, ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV422P and setting color_range
    AF_PIX_FMT_YUVJ444P, ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in favor of AV_PIX_FMT_YUV444P and setting color_range
    AF_PIX_FMT_UYVY422,  ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    AF_PIX_FMT_UYYVYY411,///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    AF_PIX_FMT_BGR8,     ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    AF_PIX_FMT_BGR4,///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    AF_PIX_FMT_BGR4_BYTE,///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    AF_PIX_FMT_RGB8,     ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    AF_PIX_FMT_RGB4,///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a byte contains two pixels, the first pixel in the byte is the one composed by the 4 msb bits
    AF_PIX_FMT_RGB4_BYTE,///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    AF_PIX_FMT_NV12,///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the UV components, which are interleaved (first byte U and the following byte V)
    AF_PIX_FMT_NV21,///< as above, but U and V bytes are swapped


    AF_PIX_FMT_YUV420P10BE = 63,///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), big-endian
    AF_PIX_FMT_YUV420P10LE,     ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb sample per 2x2 Y samples), little-endian

    AF_PIX_FMT_D3D11 = 900,
    AF_PIX_FMT_DXVA2_VLD,

    AF_PIX_FMT_APPLE_PIXEL_BUFFER = 1000,
    AF_PIX_FMT_CICADA_AF,         //framework VideoFrame
    AF_PIX_FMT_CICADA_MEDIA_CODEC,//Android mediacodec buffer index
};

/**
  * Chromaticity coordinates of the source primaries.
  * These values match the ones defined by ISO/IEC 23001-8_2013 § 7.1.
  */
enum AFColorPrimaries {
    AFCOL_PRI_RESERVED0 = 0,
    AFCOL_PRI_BT709 = 1,///< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
    AFCOL_PRI_UNSPECIFIED = 2,
    AFCOL_PRI_RESERVED = 3,
    AFCOL_PRI_BT470M = 4,///< also FCC Title 47 Code of Federal Regulations 73.682 (a)(20)

    AFCOL_PRI_BT470BG = 5,  ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
    AFCOL_PRI_SMPTE170M = 6,///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    AFCOL_PRI_SMPTE240M = 7,///< functionally identical to above
    AFCOL_PRI_FILM = 8,     ///< colour filters using Illuminant C
    AFCOL_PRI_BT2020 = 9,   ///< ITU-R BT2020
    AFCOL_PRI_SMPTE428 = 10,///< SMPTE ST 428-1 (CIE 1931 XYZ)
    AFCOL_PRI_SMPTEST428_1 = AFCOL_PRI_SMPTE428,
    AFCOL_PRI_SMPTE431 = 11, ///< SMPTE ST 431-2 (2011) / DCI P3
    AFCOL_PRI_SMPTE432 = 12, ///< SMPTE ST 432-1 (2010) / P3 D65 / Display P3
    AFCOL_PRI_JEDEC_P22 = 22,///< JEDEC P22 phosphors
    AFCOL_PRI_NB             ///< Not part of ABI
};

/**
 * Color Transfer Characteristic.
 * These values match the ones defined by ISO/IEC 23001-8_2013 § 7.2.
 */
enum AFColorTransferCharacteristic {
    AFCOL_TRC_RESERVED0 = 0,
    AFCOL_TRC_BT709 = 1,///< also ITU-R BT1361
    AFCOL_TRC_UNSPECIFIED = 2,
    AFCOL_TRC_RESERVED = 3,
    AFCOL_TRC_GAMMA22 = 4,  ///< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
    AFCOL_TRC_GAMMA28 = 5,  ///< also ITU-R BT470BG
    AFCOL_TRC_SMPTE170M = 6,///< also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC
    AFCOL_TRC_SMPTE240M = 7,
    AFCOL_TRC_LINEAR = 8,       ///< "Linear transfer characteristics"
    AFCOL_TRC_LOG = 9,          ///< "Logarithmic transfer characteristic (100:1 range)"
    AFCOL_TRC_LOG_SQRT = 10,    ///< "Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)"
    AFCOL_TRC_IEC61966_2_4 = 11,///< IEC 61966-2-4
    AFCOL_TRC_BT1361_ECG = 12,  ///< ITU-R BT1361 Extended Colour Gamut
    AFCOL_TRC_IEC61966_2_1 = 13,///< IEC 61966-2-1 (sRGB or sYCC)
    AFCOL_TRC_BT2020_10 = 14,   ///< ITU-R BT2020 for 10-bit system
    AFCOL_TRC_BT2020_12 = 15,   ///< ITU-R BT2020 for 12-bit system
    AFCOL_TRC_SMPTE2084 = 16,   ///< SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems
    AFCOL_TRC_SMPTEST2084 = AFCOL_TRC_SMPTE2084,
    AFCOL_TRC_SMPTE428 = 17,///< SMPTE ST 428-1
    AFCOL_TRC_SMPTEST428_1 = AFCOL_TRC_SMPTE428,
    AFCOL_TRC_ARIB_STD_B67 = 18,///< ARIB STD-B67, known as "Hybrid log-gamma"
    AFCOL_TRC_NB                ///< Not part of ABI
};

/**
 * YUV colorspace type.
 * These values match the ones defined by ISO/IEC 23001-8_2013 § 7.3.
 */
enum AFColorSpace {
    AFCOL_SPC_RGB = 0,  ///< order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB)
    AFCOL_SPC_BT709 = 1,///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
    AFCOL_SPC_UNSPECIFIED = 2,
    AFCOL_SPC_RESERVED = 3,
    AFCOL_SPC_FCC = 4,      ///< FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
    AFCOL_SPC_BT470BG = 5,  ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
    AFCOL_SPC_SMPTE170M = 6,///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
    AFCOL_SPC_SMPTE240M = 7,///< functionally identical to above
    AFCOL_SPC_YCGCO = 8,    ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
    AFCOL_SPC_YCOCG = AFCOL_SPC_YCGCO,
    AFCOL_SPC_BT2020_NCL = 9,         ///< ITU-R BT2020 non-constant luminance system
    AFCOL_SPC_BT2020_CL = 10,         ///< ITU-R BT2020 constant luminance system
    AFCOL_SPC_SMPTE2085 = 11,         ///< SMPTE 2085, Y'D'zD'x
    AFCOL_SPC_CHROMA_DERIVED_NCL = 12,///< Chromaticity-derived non-constant luminance system
    AFCOL_SPC_CHROMA_DERIVED_CL = 13, ///< Chromaticity-derived constant luminance system
    AFCOL_SPC_ICTCP = 14,             ///< ITU-R BT.2100-0, ICtCp
    AFCOL_SPC_NB                      ///< Not part of ABI
};

/**
 * MPEG vs JPEG YUV range.
 */
enum AFColorRange {
    AFCOL_RANGE_UNSPECIFIED = 0,
    AFCOL_RANGE_MPEG = 1,///< the normal 219*2^(n-8) "MPEG" YUV ranges
    AFCOL_RANGE_JPEG = 2,///< the normal     2^n-1   "JPEG" YUV ranges
    AFCOL_RANGE_NB       ///< Not part of ABI
};

/**
 * Location of chroma samples.
 *
 * Illustration showing the location of the first (top left) chroma sample of the
 * image, the left shows only luma, the right
 * shows the location of the chroma sample, the 2 could be imagined to overlay
 * each other but are drawn separately due to limitations of ASCII
 *
 *                1st 2nd       1st 2nd horizontal luma sample positions
 *                 v   v         v   v
 *                 ______        ______
 *1st luma line > |X   X ...    |3 4 X ...     X are luma samples,
 *                |             |1 2           1-6 are possible chroma positions
 *2nd luma line > |X   X ...    |5 6 X ...     0 is undefined/unknown position
 */
enum AFChromaLocation {
    AFCHROMA_LOC_UNSPECIFIED = 0,
    AFCHROMA_LOC_LEFT = 1,   ///< MPEG-2/4 4:2:0, H.264 default for 4:2:0
    AFCHROMA_LOC_CENTER = 2, ///< MPEG-1 4:2:0, JPEG 4:2:0, H.263 4:2:0
    AFCHROMA_LOC_TOPLEFT = 3,///< ITU-R 601, SMPTE 274M 296M S314M(DV 4:1:1), mpeg2 4:2:2
    AFCHROMA_LOC_TOP = 4,
    AFCHROMA_LOC_BOTTOMLEFT = 5,
    AFCHROMA_LOC_BOTTOM = 6,
    AFCHROMA_LOC_NB///< Not part of ABI
};

typedef enum InterlacedType_t {
    InterlacedType_UNKNOWN = -1,
    InterlacedType_NO,
    InterlacedType_YES
} InterlacedType;

typedef struct {
    int nChannles;
    int sample_rate;
    enum AFCodecID codec;
    enum AFSampleFormat sample_fmt;
    uint64_t channel_layout;
    int frame_size;
    uint8_t *extradata;
    int extradata_size;
    int bits_per_coded_sample;
    int nb_frame;

} audio_info;

typedef enum {
    picture_cache_type_unknown = -1,
    picture_cache_type_soft,
    picture_cache_type_cannot

} picture_cache_type;

typedef struct {
    int pix_fmt;
    int width;
    int height;
    picture_cache_type cache_type;

} video_info;

/**
 * The stream is stored in the file as an attached picture/"cover art" (e.g.
 * APIC frame in ID3v2). The single packet associated with it will be returned
 * among the first few packets read from the file unless seeking takes place.
 * It can also be accessed at any time in AVStream.attached_pic.
 */
#define AV_DISPOSITION_ATTACHED_PIC      0x0400

typedef struct Source_meta {
    char *key;
    char *value;
    struct Source_meta *next;
} Source_meta;


typedef struct {
    Stream_type type;
    int64_t duration;
    enum AFCodecID codec;
    uint32_t codec_tag;
    int index;
    int nb_index_entries;
    void *cicada_codec_context;
    int cicada_codec_context_size;

    char *title;
    char *language;
    int64_t seeked_time;

    int disposition; /**< AV_DISPOSITION_* bit field */

    // TODO:  use union
    //audio
    int channels;
    uint64_t channel_layout;
    int samplerate;
    int frame_size;
    int profile;
    int bits_per_coded_sample;
    enum AFSampleFormat sample_fmt;

    //video only
    int width;
    int height;
    int rotate;
    int displayWidth;
    int displayHeight;
    double avg_fps;
    enum AFPixelFormat pixel_fmt;
    /**
 * Video only. Additional colorspace characteristics.
 */
    enum AFColorRange color_range;
    enum AFColorPrimaries color_primaries;
    enum AFColorTransferCharacteristic color_trc;
    enum AFColorSpace color_space;
    enum AFChromaLocation chroma_location;

    int pid;
    int no_program;
    int attached_pic;
    uint8_t *extradata;
    int extradata_size;
    InterlacedType interlaced;

    char *lang;
    uint64_t bandwidth;

    char *description;

    Source_meta *meta;

    //add for stand drm(WideVine,FairPlay...)
    char* keyUrl;
    char* keyFormat;

    float ptsTimeBase;

} Stream_meta;


enum color_space {
    COLOR_SPACE_UNSPECIFIED = 0,
    COLOR_SPACE_BT709 = 1,
    COLOR_SPACE_BT601 = 2,
    COLOR_SPACE_BT2020 = 6
};

enum color_range {
    COLOR_RANGE_UNSPECIFIED = 0,
    COLOR_RANGE_FULL = 1,
    COLOR_RANGE_LIMITIED = 2,
};

#define MAX_AUDIO_FRAME_SIZE 192000

typedef int  (*decoder_buf_callback)(unsigned char *buffer[], int nb_samples, int line_size,
                                     long long pts,
                                     void *CbpHandle);

typedef int  (*decoder_buf_callback_video)(unsigned char *buffer[], int linesize[], long long pts,
                                           int type,
                                           void *CbpHandle);

enum dec_flag {
    dec_flag_dummy,
    dec_flag_hw,
    dec_flag_sw,
    dec_flag_out,
    dec_flag_direct,
    dec_flag_adaptive,
    dec_flag_passthrough_info,
    // adjust setting to output frames as soon as possiable.
    dec_flag_output_frame_asap,
};
#define DECFLAG_DUMMY  1u << dec_flag_dummy
#define DECFLAG_HW     (1u << dec_flag_hw)
#define DECFLAG_SW     (1u << dec_flag_sw)
#define DECFLAG_OUT    (1u << dec_flag_out)
#define DECFLAG_DIRECT (1u << dec_flag_direct)
#define DECFLAG_ADAPTIVE (1u << dec_flag_adaptive)
#define DECFLAG_PASSTHROUGH_INFO (1 << dec_flag_passthrough_info)
#define DECFLAG_OUTPUT_FRAME_ASAP (1u << dec_flag_output_frame_asap)

typedef struct mediaFrame_t mediaFrame;

typedef void (*FrameRelease)(void *arg, mediaFrame *pFrame);

typedef enum {
    SKIP_OUTPUT_FALSE = 0,
    SKIP_OUTPUT_TRUE = 1,
} SKIP_OUTPUT;

typedef struct mediaFrame_t {
    SKIP_OUTPUT skipOutput;
    uint8_t *pBuffer;
    int size;
    int streamIndex;
    int64_t pts;
    int64_t dts;
    int flag;
    int duration;
    int64_t pos;

    int64_t startTime;
    FrameRelease release;
    void *release_arg;

    void *pAppending;

    int width;
    int height;
} mediaFrame;

enum callback_cmd {
    callback_cmd_get_source_meta,
    callback_cmd_get_sink_meta,
    callback_cmd_get_frame
};

#define MKTAG(a, b, c, d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define CICADAERRTAG(a, b, c, d) (-(int)MKTAG(a, b, c, d))
#define SEGEND   CICADAERRTAG(0xF9,'S','E','D')

#ifdef WIN32

#if (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#include <d3d11.h>
typedef struct CicadaD3D11VADeviceContext {
    /**
     * Device used for texture creation and access. This can also be used to
     * set the libavcodec decoding device.
     *
     * Must be set by the user. This is the only mandatory field - the other
     * device context fields are set from this and are available for convenience.
     *
     * Deallocating the AVHWDeviceContext will always release this interface,
     * and it does not matter whether it was user-allocated.
     */
    ID3D11Device        *device;

    /**
     * If unset, this will be set from the device field on init.
     *
     * Deallocating the AVHWDeviceContext will always release this interface,
     * and it does not matter whether it was user-allocated.
     */
    ID3D11DeviceContext *device_context;

    /**
     * If unset, this will be set from the device field on init.
     *
     * Deallocating the AVHWDeviceContext will always release this interface,
     * and it does not matter whether it was user-allocated.
     */
    ID3D11VideoDevice   *video_device;

    /**
     * If unset, this will be set from the device_context field on init.
     *
     * Deallocating the AVHWDeviceContext will always release this interface,
     * and it does not matter whether it was user-allocated.
     */
    ID3D11VideoContext  *video_context;

    /**
     * Callbacks for locking. They protect accesses to device_context and
     * video_context calls. They also protect access to the internal staging
     * texture (for av_hwframe_transfer_data() calls). They do NOT protect
     * access to hwcontext or decoder state in general.
     *
     * If unset on init, the hwcontext implementation will set them to use an
     * internal mutex.
     *
     * The underlying lock must be recursive. lock_ctx is for free use by the
     * locking implementation.
     */
    void (*lock)(void *lock_ctx);
    void (*unlock)(void *lock_ctx);
    void *lock_ctx;
} CicadaD3D11VADeviceContext;
#endif
#endif

typedef enum AFHWDeviceType {
    AF_HWDEVICE_TYPE_UNKNOWN = -1,
    AF_HWDEVICE_TYPE_D3D11VA = 0,
} AFHWDeviceType;


#endif
