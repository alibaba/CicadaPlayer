
#ifndef CicadaFrameInfo_h
#define CicadaFrameInfo_h

#import <Foundation/Foundation.h>
#import <CoreVideo/CoreVideo.h>

typedef enum  CicadaFrameType : NSUInteger{
    Cicada_FrameType_Unknown,
    Cicada_FrameType_Video,
    Cicada_FrameType_Audio
} CicadaFrameType;

typedef  enum   CicadaSampleFormat : NSInteger {
    CICADA_SAMPLE_FMT_NONE = -1,
    CICADA_SAMPLE_FMT_U8,  ///< unsigned 8 bits
    CICADA_SAMPLE_FMT_S16, ///< signed 16 bits
    CICADA_SAMPLE_FMT_S32, ///< signed 32 bits
    CICADA_SAMPLE_FMT_FLT, ///< float
    CICADA_SAMPLE_FMT_DBL, ///< double

    CICADA_SAMPLE_FMT_U8P,  ///< unsigned 8 bits, planar
    CICADA_SAMPLE_FMT_S16P, ///< signed 16 bits, planar
    CICADA_SAMPLE_FMT_S32P, ///< signed 32 bits, planar
    CICADA_SAMPLE_FMT_FLTP, ///< float, planar
    CICADA_SAMPLE_FMT_DBLP, ///< double, planar

    CICADA_SAMPLE_FMT_NB, ///< Number of sample formats. DO NOT USE if linking dynamically

} CicadaSampleFormat;

typedef enum  CicadaPixelFormat : NSInteger {
    CICADA_PIX_FMT_NONE = -1,
    CICADA_PIX_FMT_YUV420P, ///< planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    CICADA_PIX_FMT_YUYV422, ///< packed YUV 4:2:2, 16bpp, Y0 Cb Y1 Cr
    CICADA_PIX_FMT_RGB24,   ///< packed RGB 8:8:8, 24bpp, RGBRGB...
    CICADA_PIX_FMT_BGR24,   ///< packed RGB 8:8:8, 24bpp, BGRBGR...
    CICADA_PIX_FMT_YUV422P, ///< planar YUV 4:2:2, 16bpp, (1 Cr & Cb sample per 2x1 Y samples)
    CICADA_PIX_FMT_YUV444P, ///< planar YUV 4:4:4, 24bpp, (1 Cr & Cb sample per 1x1 Y samples)
    CICADA_PIX_FMT_YUV410P, ///< planar YUV 4:1:0,  9bpp, (1 Cr & Cb sample per 4x4 Y samples)
    CICADA_PIX_FMT_YUV411P, ///< planar YUV 4:1:1, 12bpp, (1 Cr & Cb sample per 4x1 Y samples)
    CICADA_PIX_FMT_GRAY8, ///<        Y        ,  8bpp
    CICADA_PIX_FMT_MONOWHITE, ///<        Y        ,  1bpp, 0 is white, 1 is black, in each
    ///<        byte pixels are ordered from the msb to the lsb
    CICADA_PIX_FMT_MONOBLACK, ///<        Y        ,  1bpp, 0 is black, 1 is white, in each
    ///<        byte pixels are ordered from the msb to the lsb
    CICADA_PIX_FMT_PAL8, ///< 8 bits with AV_PIX_FMT_RGB32 palette
    CICADA_PIX_FMT_YUVJ420P, ///< planar YUV 4:2:0, 12bpp, full scale (JPEG), deprecated in
    ///< favor of AV_PIX_FMT_YUV420P and setting color_range
    CICADA_PIX_FMT_YUVJ422P, ///< planar YUV 4:2:2, 16bpp, full scale (JPEG), deprecated in
    ///< favor of AV_PIX_FMT_YUV422P and setting color_range
    CICADA_PIX_FMT_YUVJ444P, ///< planar YUV 4:4:4, 24bpp, full scale (JPEG), deprecated in
    ///< favor of AV_PIX_FMT_YUV444P and setting color_range
    CICADA_PIX_FMT_UYVY422,   ///< packed YUV 4:2:2, 16bpp, Cb Y0 Cr Y1
    CICADA_PIX_FMT_UYYVYY411, ///< packed YUV 4:1:1, 12bpp, Cb Y0 Y1 Cr Y2 Y3
    CICADA_PIX_FMT_BGR8,      ///< packed RGB 3:3:2,  8bpp, (msb)2B 3G 3R(lsb)
    CICADA_PIX_FMT_BGR4, ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1B 2G 1R(lsb), a
    ///< byte contains two pixels, the first pixel in the byte is the
    ///< one composed by the 4 msb bits
    CICADA_PIX_FMT_BGR4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1B 2G 1R(lsb)
    CICADA_PIX_FMT_RGB8,      ///< packed RGB 3:3:2,  8bpp, (msb)2R 3G 3B(lsb)
    CICADA_PIX_FMT_RGB4, ///< packed RGB 1:2:1 bitstream,  4bpp, (msb)1R 2G 1B(lsb), a
    ///< byte contains two pixels, the first pixel in the byte is the
    ///< one composed by the 4 msb bits
    CICADA_PIX_FMT_RGB4_BYTE, ///< packed RGB 1:2:1,  8bpp, (msb)1R 2G 1B(lsb)
    CICADA_PIX_FMT_NV12, ///< planar YUV 4:2:0, 12bpp, 1 plane for Y and 1 plane for the
    ///< UV components, which are interleaved (first byte U and the
    ///< following byte V)
    CICADA_PIX_FMT_NV21, ///< as above, but U and V bytes are swapped

    CICADA_PIX_FMT_YUV420P10BE = 63, ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb
    ///< sample per 2x2 Y samples), big-endian
    CICADA_PIX_FMT_YUV420P10LE, ///< planar YUV 4:2:0, 15bpp, (1 Cr & Cb
    ///< sample per 2x2 Y samples), little-endian

    CICADA_PIX_FMT_D3D11 = 900,
    CICADA_PIX_FMT_DXVA2_VLD,

    CICADA_PIX_FMT_APPLE_PIXEL_BUFFER = 1000,
    CICADA_PIX_FMT_CICADA_AF,          // framework VideoFrame
    CICADA_PIX_FMT_CICADA_MEDIA_CODEC, // Android mediacodec buffer index

} CicadaPixelFormat;

OBJC_EXPORT
@interface CicadaFrameInfo : NSObject

@property(nonatomic, assign) CicadaFrameType frameType;
@property(nonatomic, assign) long pts;
@property(nonatomic, assign) long duration;
@property(nonatomic, assign) BOOL key;
@property(nonatomic, assign) long timePosition;

@property(nonatomic, assign) CicadaSampleFormat audio_format;
@property(nonatomic, assign) int audio_nb_samples;
@property(nonatomic, assign) int audio_channels;
@property(nonatomic, assign) int audio_sample_rate;
@property(nonatomic, assign) uint64_t audio_channel_layout;
@property(nonatomic, assign) int  audio_data_lineNum;
@property(nonatomic, assign) uint8_t ** audio_data;
@property(nonatomic, assign) int audio_data_lineSize;


@property(nonatomic, assign) CicadaPixelFormat video_format;
@property(nonatomic, assign) int video_width;
@property(nonatomic, assign) int video_height;
@property(nonatomic, assign) int video_rotate;
@property(nonatomic, assign) double video_dar;
@property(nonatomic, assign) size_t video_crop_top;
@property(nonatomic, assign) size_t video_crop_bottom;
@property(nonatomic, assign) size_t video_crop_left;
@property(nonatomic, assign) size_t video_crop_right;
@property(nonatomic, assign) int video_colorRange;
@property(nonatomic, assign) int video_colorSpace;
@property(nonatomic, assign) int  video_data_lineNum;
@property(nonatomic, assign) int* video_data_lineSize;
@property(nonatomic, assign) uint8_t ** video_data_addr;
@property(nonatomic, assign) CVPixelBufferRef video_pixelBuffer;

@end


#endif /* CicadaFrameInfo_h */
